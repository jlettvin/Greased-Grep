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
R"Synopsis(Usage: gg [-s] [+|-]{str} [[+|-]{str}...] {path} 
Greased Grep search for files having (case insensitive):
    all instances of +{str} or {str} and
	no  instances of -{str} instances in
	files found along {path}

        {str} are simple strings (no regex).
        {str} may be single-quoted to avoid shell interpretation.

    [+]{str}
        add accept string (+ optional)

    -{str}
        add reject string

    -s
        suppress permission denied errors

    {path}
        top directory for recursive search

Examples:

    gg include /usr/local/src
        # find all files having the string 'inlude' in /usr/local/src

    gg '#include <experimental/filesystem>' /usr/local/src
        # find all files having the quoted string in /usr/local/src

    gg copyright -Lettvin .
        # Find all files with missing or other than Lettvin copyright.
)Synopsis";

// TODO find bug for when m_table is not reserved
// TODO measure performance against fgrep/ack/ag

#include <experimental/filesystem>

#include <fmt/printf.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <string_view>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>

static void synopsis (const char* a_message = nullptr);

namespace Lettvin
{
	using namespace std;
	namespace fs = std::experimental::filesystem;
	typedef unsigned char u08_t;
	typedef int           i24_t;

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition element.
	///
	/// constructor/getters/setters for atomic element unit.
	//__________________________________________________________________________
	class
	Atom
	//__________________________________________________________________________
	{
	//------
	public:
	//------
		Atom () {}                                               ///< ctor
		u08_t tgt (         ) const { return  m_the.state.tgt; } ///< tgt getter
		i24_t str (         ) const { return  m_the.state.str; } ///< str getter
		void  tgt (u08_t a_tgt)     { m_the.state.tgt = a_tgt; } ///< tgt setter
		void  str (i24_t a_str)     { m_the.state.str = a_str; } ///< str setter
	//------
	private:
	//------
		union {
			unsigned short integral;                   ///< unused name
			struct { i24_t str:24; u08_t tgt; } state; ///< strtern/state ids
		} m_the
		{.integral=0};
	}; // class Atom

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition plane of 256 Atoms
	///
	/// constructor/indexer
	//__________________________________________________________________________
	class
	State
	//__________________________________________________________________________
	{
	//------
	public:
	//------
		State () {}                                                ///< ctor
		Atom& operator[] (u08_t a_off) { return m_handle[a_off]; } ///< indexer
	//------
	private:
	//------
		Atom m_handle[256];
	}; // class State

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief vector of state-transition planes sufficient to enable search
	///
	/// constructor/indexer
	//__________________________________________________________________________
	class
	Table
	//__________________________________________________________________________
	{
	//------
	public:
	//------

		//----------------------------------------------------------------------
		/// @brief Table ctor (reserve many, instance 2)
		Table ()
		//----------------------------------------------------------------------
		{
			// TODO find bug for when m_table is not reserved
			m_table.reserve (256);
			operator++ ();
			operator++ ();
		} // ctor

		//----------------------------------------------------------------------
		/// @brief indexer
		State&
		operator[] (u08_t a_offset)
		//----------------------------------------------------------------------
		{
			return m_table[a_offset];
		} // operator[]

		//----------------------------------------------------------------------
		/// @brief add State planes to vector
		void
		operator++ ()
		//----------------------------------------------------------------------
		{
			m_table.resize (m_table.size () + 1);
		} // operator++

		//----------------------------------------------------------------------
		/// @brief return current size of vector
		size_t
		size ()
		//----------------------------------------------------------------------
		{
			return m_table.size ();
		} // size

	//------
	private:
	//------

		vector<State> m_table; ///< State tables

	}; // class Table

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	//__________________________________________________________________________
	/// @brief GreasedGrep implements overall state-transition operations
	class
	GreasedGrep : Table
	//__________________________________________________________________________
	{
	//------
	public:
	//------

		//----------------------------------------------------------------------
		/// @brief ctor
		GreasedGrep (int a_argc, char** a_argv) // ctor
		//----------------------------------------------------------------------
		{
			while (--a_argc) ingest (*++a_argv);
		} // ctor

		//----------------------------------------------------------------------
		/// @brief ftor
		void
		operator() ()
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
				const Atom& element (m_state1[i]);
				auto t{element.tgt ()};
				if (t) m_firsts += static_cast<u08_t>(i);
			}

			// Check for valid directory
			if (!fs::is_directory (m_directory))
			{
				synopsis ("last arg must be dir");
			}

			// Find files and search contents
			walk (m_directory);

		} // operator ()

	//------
	private:
	//------

		//----------------------------------------------------------------------
		/// @brief ingest inserts state-transition table data
		void
		ingest (string_view a_str)
		//----------------------------------------------------------------------
		{
			if (a_str == "-s")
			{
				m_suppress = true;
				return;
			}
			if (m_directory.size ())
			{
				//static const char* direction[2]{"ACCEPT", "REJECT"};
				auto from             {m_root};
				auto next             {from};
				auto to               {from};
				vector<u08_t> last    {0,0};
				string_view candidate {m_directory};
				char c0               {candidate[0]};
				bool rejecting        {c0 == '-'};
				auto& field           {rejecting ? m_reject : m_accept};

				m_noreject &= !rejecting; ///< aids in optimizing inner loop

				i24_t sign{rejecting?-1:+1};
				i24_t id{sign*static_cast<i24_t> (field.size ())};
				candidate.remove_prefix ((c0=='+' || c0=='-') ? 1 : 0);

				field.push_back (candidate);

				// Insert a_str into state transition tree
				for (char u:candidate)
				{
					last[0] = static_cast<char> (toupper (u));
					last[1] = static_cast<char> (tolower (u));
#if 0
					for (auto c:last)
					{
						Atom& element{operator[] (from)[c]};
						to = element.tgt ();
						next = from;
						if (to) { from = to; }
						else { from = Table::size (); operator++ (); }
						element.tgt (from);
					}
#else
					Atom& ELEMENT{operator[] (from)[last[0]]};
					Atom& element{operator[] (from)[last[1]]};
					to = element.tgt ();
					next = from;
					if (to) { from = to; }
					else { from = Table::size (); operator++ (); }
					element.tgt (from);
					ELEMENT.tgt (from);
#endif
				}
				for (auto c:last) operator[] (next)[c].str (id);
			}
			m_directory = a_str;
		} // ingest

		//----------------------------------------------------------------------
		/// @brief find and report found strings
		///
		/// when reject list is empty, terminate on completion of accept list
		/// when reject list is non-empty, terminate on first reject
		void
		search (const char* a_filename, void* a_pointer, auto a_filesize)
		//----------------------------------------------------------------------
		{
			set<i24_t> accepted  {0};
			set<i24_t> rejected  {};
			string_view contents (static_cast<char*> (a_pointer), a_filesize);
			size_t begin = contents.find_first_of (m_firsts);
			bool done{false};
			// outer loop (skip optimization)
			while (begin != string_view::npos && !done)
			{
				contents.remove_prefix (begin);
				auto st{1};
				auto pt{0};
				// inner loop (Finite State Machine optimization)
				for (auto c: contents)
				{
					auto element{operator[] (st)[c]};
					st = element.tgt ();
					pt = element.str ();
					if (pt) {
						if (pt < 0) return; ///< Immediate rejection
						// If not immediate rejection, add to rejected list
						auto& chose{(pt>0)?accepted:rejected};
						chose.insert (pt);
						bool full_accept{m_accept.size () == accepted.size ()};
						// completion optimization
						done = (m_noreject && full_accept);
					}
					if (done || !st) break;
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
		/// @brief map file into memory and call search
		///
		/// https://techoverflow.net/2013/08/21/a-simple-mmap-readonly-example/
		void
		mapped_search (const char* a_filename)
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

		//----------------------------------------------------------------------
		/// @brief walk organizes search for strings in memory-mapped file
		void
		walk (const fs::path& a_path)
		//----------------------------------------------------------------------
		{
			try
			{
				for (auto& element: fs::recursive_directory_iterator (a_path))
				{
					const char* filename{element.path ().c_str ()};
					if (fs::is_regular_file (element.status ()))
					{
						try
						{
							mapped_search (filename);
						}
						catch (...)
						{
							if (!m_suppress)
							{
								printf ("gg: %s file Permission denied\n",
										filename);
							}
						}
					}
				}
			}
			catch (...)
			{
				if (!m_suppress)
				{
					printf ("gg: %s dir Permission denied\n",
							a_path.filename ().c_str ());
				}
			}
		} // walk

		//dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
		vector<string_view> m_accept {{""}}; ///< list of accept {str} args
		vector<string_view> m_reject {{""}}; ///< list of reject {str} args
		u08_t m_root{1};                     ///< syntax tree root plane number
		size_t m_debug{0};                   ///< turns on verbosity
		bool m_noreject{true};               ///< are there reject strings?
		bool m_suppress{false};              ///< suppress error messages

		string_view m_directory;
		string m_firsts;                     ///< string of {arg} first letters
		State& m_state1{operator[] (m_root)};///< root state plane
	}; // class GreasedGrep

} // namespace Lettvin

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

//------------------------------------------------------------------------------
/// @brief synopsis (document usage in case of failure)
static
void
synopsis (const char* a_message)
//------------------------------------------------------------------------------
{
	if (a_message != nullptr) { printf ("ERROR: %s\n\n", a_message); }
	printf ("%s", Synopsis);
	exit (1);
} // synopsis

//------------------------------------------------------------------------------
/// @brief main (program execution entrypoint)
int
main (int argc, char** argv)
//------------------------------------------------------------------------------
{
	Lettvin::GreasedGrep gg (argc, argv);
	gg ();
	return 0;
} // main
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

