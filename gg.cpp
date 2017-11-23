/*_____________________________________________________________________________
            The MIT License (https://opensource.org/licenses/MIT)

        Copyright (c) 2017, Jonathan D. Lettvin, All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
_____________________________________________________________________________*/

static const char* Synopsis =
R"Synopsis(Usage: gg [+|-]{str} [[+|-]{str}...] {path} 
Greased Grep search for files having (case insensitive):
    all instances of +{str} or {str} and
	no  instances of -{str} instances in
	files found along {path}

{str} are simple strings (no regex).
{str} may be single-quoted to avoid shell interpretation.

Examples:

    gg include /usr/local/src

    gg '#include <experimental/filesystem>' /usr/local/src

    gg copyright -Lettvin .
        # Find all files with missing or other than Lettvin copyright.
)Synopsis";

// TODO check that the final arg is a directory, otherwise synopsis.
// TODO handle canonicalization problem.
// TODO find bug for when m_table is not reserved
// TODO when reject list is empty, terminate of completion of accept list
// TODO when reject list is non-empty, terminate on first reject

#include <experimental/filesystem>
#include <fmt/printf.h>
#include <sys/types.h>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <fcntl.h>
#include <string>
#include <vector>
#include <set>

namespace greased_grep
{
	using namespace std;
	namespace fs = std::experimental::filesystem;
	typedef unsigned char u08_t;
	typedef int           i24_t;

	//--------------------------------------------------------------------------
	/// @brief synopsis (document usage in case of failure)
	void synopsis (const char* a_message = nullptr)
	//--------------------------------------------------------------------------
	{
		if (a_message != nullptr) { printf ("ERROR: %s\n\n", a_message); }
		printf ("%s", Synopsis);
		exit (1);
	}

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition element.
	class Element
	{
	//------
	public:
	//------
		Element () {}
		u08_t tgt (       ) const { return m_the.state.tgt; }
		i24_t pat (       ) const { return m_the.state.pat;  }
		void  tgt (u08_t a_c)       { m_the.state.tgt = a_c; }
		void  pat (i24_t a_c)       { m_the.state.pat = a_c; }
	//------
	private:
	//------
		union {
			unsigned short integral;                   ///< unused name
			struct { i24_t pat:24; u08_t tgt; } state; ///< pattern/state ids
		} m_the
		{.integral=0};
	};

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	class State
	{
	//------
	public:
	//------
		State () {}
		Element& operator[] (u08_t a_offset) { return m_handle[a_offset]; }
	//------
	private:
	//------
		Element m_handle[256];
	};

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	class Table
	{
	//------
	public:
	//------
		Table ()
		{
			// TODO find bug for when m_table is not reserved
			m_table.reserve (256);
			operator++ ();
			operator++ ();
		}

		State& operator[] (u08_t a_offset) { return m_table[a_offset]; }

		void operator++ () { m_table.resize (m_table.size () + 1); }

		size_t size () { return m_table.size (); }
	//------
	private:
	//------
		vector<State> m_table;
	};

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	class GreasedGrep : Table
	{
	//------
	public:
	//------

		//----------------------------------------------------------------------
		GreasedGrep (int a_argc, char** a_argv)
		//----------------------------------------------------------------------
		{
			while (--a_argc) ingest (*++a_argv);
		} // ctor

		//----------------------------------------------------------------------
		void operator() ()
		//----------------------------------------------------------------------
		{
			// Validate ingested args
			if (m_accept.size () < 2)
			{
				synopsis ("specify at least one accept str");
			}

			// Initialize firsts to enable buffer skipping
			for (size_t i=0; i<256; ++i)
			{
				const Element& element (m_state1[i]);
				auto t{element.tgt ()};
				if (t) m_firsts += static_cast<u08_t>(i);
			}

			// Prepare directory for use with std::experimental/filesystem.
			m_dir = fs::canonical (m_dir);

			// Find files and search contents
			walk (m_dir);

		} // operator ()

	//------
	private:
	//------

		//----------------------------------------------------------------------
		void ingest (string_view a_str)
		//----------------------------------------------------------------------
		{
			if (m_directory.size ())
			{
				auto from             {m_root};
				auto next             {from};
				auto to               {from};
				u08_t last[2]         {0,0};
				string_view candidate {m_directory};
				char c0               {candidate[0]};
				bool rejecting        {c0 == '-'};
				auto& field           {rejecting ? m_reject : m_accept};

				i24_t sign{rejecting?-1:+1};
				i24_t id{sign*static_cast<i24_t> (field.size ())};
				candidate.remove_prefix ((c0=='+' || c0=='-') ? 1 : 0);

				field.push_back (candidate);

				// Insert a_str into state transition tree
				for (char u:candidate)
				{
					char C{static_cast<char> (toupper (u))};
					char c{static_cast<char> (tolower (u))};
					last[0] = C;
					last[1] = c;
					Element& element{operator[] (from)[c]};
					Element& ELEMENT{operator[] (from)[C]};
					to = element.tgt ();
					next = from;
					if (to) { from = to; }
					else { from = Table::size (); operator++ (); }
					//from = to ? to : ++size;
					element.tgt (from);
					ELEMENT.tgt (from);
				}
				operator[] (next)[last[0]].pat (id);
				operator[] (next)[last[1]].pat (id);
			}
			m_directory = a_str;
		} // ingest

		//----------------------------------------------------------------------
		void walk (const fs::path& a_path)
		//----------------------------------------------------------------------
		{
			if (fs::exists (a_path) && fs::is_directory (a_path))
			{
				for (const auto& element:fs::recursive_directory_iterator (a_path))
				{
					if (fs::is_directory (element.status ()))
					{
						auto name = element.path ().filename ();
						walk (element);
					}
					else if (fs::is_regular_file (element.status ()))
					{
						auto name = element.path ().filename ();
						auto canon = name;
						try
						{
							canon = fs::canonical (name);
						}
						catch(...)
						{
							if (m_debug)
							{
								printf ("\tException canonicalizing %s\n",
										name.c_str ());
							}
						}
						mapped_search (canon.c_str ());
					}
				}
			}
		} // walk

		//----------------------------------------------------------------------
		void search (const char* a_filename, void* a_pointer, auto a_filesize)
		//----------------------------------------------------------------------
		{
			set<i24_t> accepted  {0};
			set<i24_t> rejected  {};
			string_view contents (static_cast<char*> (a_pointer), a_filesize);
			size_t begin = contents.find_first_of (m_firsts);
			while (begin != string_view::npos)
			{
				contents.remove_prefix (begin);
				auto st{1};
				auto pt{0};
				for (auto c: contents)
				{
					auto element{operator[] (st)[c]};
					st = element.tgt ();
					pt = element.pat ();
					if (pt != 0) {
						auto& chose{(pt>0)?accepted:rejected};
						chose.insert (pt);
					}
					if (!st) break;
				}
				contents.remove_prefix (1);
				begin = contents.find_first_of (m_firsts);
			}

			// Report files having all accepteds and no rejecteds.
			if (!rejected.size () && accepted.size () == m_accept.size ())
			{
				printf ("%s\n", a_filename);
			}
		} // search

		//----------------------------------------------------------------------
		// https://techoverflow.net/2013/08/21/a-simple-mmap-readonly-example/
		void mapped_search (const char* a_filename)
		//----------------------------------------------------------------------
		{
			struct stat st;
			stat (a_filename, &st);
			auto filesize{st.st_size};
			int fd = open (a_filename, O_RDONLY, 0);
			if (fd >= 0)
			{
				void* contents = mmap (
						NULL,
						filesize,
						PROT_READ,					// Optimize out dirty pages
						MAP_PRIVATE | MAP_POPULATE,	// Allow preload
						fd,
						0);

				if (contents != MAP_FAILED)
				{
					search (a_filename, contents, filesize);
					int rc = munmap (contents, filesize);
					if (rc != 0) synopsis ("munmap failed");
				}

				close (fd);
			}
		} // mapped_search

		vector<string_view> m_accept {{""}}; ///< list of accept {str} args
		vector<string_view> m_reject {{""}}; ///< list of reject {str} args
		u08_t m_root{1};                     ///< syntax tree root plane number
		size_t m_debug{0};                   ///< turns on verbosity

		string_view m_directory;
		fs::path m_dir;
		string m_firsts;                     ///< string of {arg} first letters
		State& m_state1{operator[] (m_root)};///< root state plane
	}; // class GreasedGrep
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
int main (int argc, char** argv)
{
	greased_grep::GreasedGrep gg (argc, argv);
	gg ();
	return 0;
} // main
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

