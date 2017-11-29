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
R"Synopsis(Greased Grep version %u.%u.%u

USAGE: gg [-c] [-s] [+|-]{str} [[+|-]{str}...] {path} 

Greased Grep search for files having (case insensitive):
    all instances of +{str} or {str} and
	no  instances of -{str} instances in
	files found along {path}

        {str} are simple strings (no regex).
        {str} may be single-quoted to avoid shell interpretation.

ARGUMENTS:

    [+]{str}      # add accept string (+ optional)

    -{str}        # add reject string

    {path}        # top directory for recursive search

OPTIONS:

    -c            # case sensitive search

    -n            # use nibbles (lower memory use half-speed search)

    -s            # suppress permission denied errors

EXAMPLES:

    gg include /usr/local/src
        # find all files having the string 'inlude' in /usr/local/src

    gg '#include <experimental/filesystem>' /usr/local/src
        # find all files having the quoted string in /usr/local/src

    gg copyright -Lettvin .
        # Find all files with missing or other than Lettvin copyright.
)Synopsis";

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
// TODO find bug for when m_table is not reserved
// TODO measure performance against fgrep/ack/ag
// TODO ingest args with ctor but compile strs at beginning of ftor
// TODO use memcmp for unique final string
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
// Turn GG_COMPILE true to run compile AFTER command-line processing
// For now, tests fail when GG_COMPILE is true
#define GG_COMPILE false

//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

//..............................................................................
#include <experimental/filesystem> // recursive directory walk

//..............................................................................
#include <fmt/printf.h>            // modern printf

//..............................................................................
#include <sys/mman.h>              // Memory mapping
#include <sys/stat.h>              // File status via descriptor

//..............................................................................
#include <unistd.h>                // file descriptor open/write/close
#include <fcntl.h>                 // file descriptor open O_RDONLY

//..............................................................................
#include <string_view>             // Improve performance on mmap of file
#include <iostream>                // sync_with_stdio (mix printf with cout)
#include <iomanip>                 // setw and other cout formatting

//..............................................................................
#include <string>                  // container
#include <vector>                  // container
#include <set>                     // container

//..............................................................................
#include "gg.h"                    // version

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace Lettvin
{
	namespace fs = std::experimental::filesystem;

	using namespace std;  // No naming collisions in this small namespace

	typedef unsigned char u08_t;
	typedef int           i24_t;

	//--------------------------------------------------------------------------
	/// @brief synopsis (document usage in case of failure)
	static
	void
	synopsis (const char* a_message)
	//--------------------------------------------------------------------------
	{
		if (a_message != nullptr)
		{
			printf ("ERROR: %s\n\n", a_message);
		}
		printf (Synopsis, s_version.major, s_version.minor, s_version.build);
		exit (1);
	} // synopsis

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
		State () : m_handle (s_size)      {                         } ///< ctor
		Atom& operator[] (u08_t a_off)    { return m_handle[a_off]; } ///< index
		static void nibbles (bool a_flag) { s_size = a_flag ? 16 : 256; }
	//------
	private:
	//------
		vector<Atom> m_handle;
		static size_t s_size;
	}; // class State
	size_t State::s_size{256};

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
		static
		void
		nibbles (bool a_flag)
		//----------------------------------------------------------------------
		{
			s_nibbles = a_flag;
			s_prefill = s_nibbles ? 2 : 1;
			s_mask    = s_nibbles ? 0x0fULL : 0xffULL;
		} // nibbles

		//----------------------------------------------------------------------
		/// @brief Table ctor (reserve many, instance 2)
		Table ()
		//----------------------------------------------------------------------
		{
			// TODO find bug for when m_table is not reserved
			m_table.reserve (s_prefill*256);
			for (size_t i=0; i < s_prefill; ++i)
			{
				operator++ ();
				operator++ ();
			}
		} // ctor

		//----------------------------------------------------------------------
		/// @brief indexer
		State&
		operator[] (u08_t a_offset)
		//----------------------------------------------------------------------
		{
			return m_table[a_offset & s_mask];
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

		//----------------------------------------------------------------------
		/// @brief debug utility for displaying the entire table
		ostream&
		show_tables (ostream& a_os)
		//----------------------------------------------------------------------
		{
			size_t COLS{s_nibbles ? 4ULL : 16ULL};
			size_t ROWS{s_nibbles ? 4ULL : 16ULL};
			for (size_t state=0; state < m_table.size (); ++state)
			{
				a_os << "\tPLANE: " << state << endl;
				auto& plane{m_table[state]};
				a_os << ' ' << string (5*COLS, '_') << '\n';
				for (size_t row=0; row < ROWS*COLS; row+=COLS)
				{
					a_os << '|';
					for (size_t col=0; col < COLS; ++col)
					{
						char id{static_cast<char>(row+col)};
						Atom& entry{plane[id]};
						int tgt{static_cast<int>(entry.tgt ())};
						char gra{id>=' '&&id<='~'?id:' '};
						if (tgt) a_os << gra << setw(3) << tgt << ' ';
						else     a_os << "     ";
					}
					a_os << "|\n|";
					for (size_t col=0; col < COLS; ++col)
					{
						Atom& entry{plane[row+col]};
						int str{static_cast<int>(entry.str ())};
						if (str) a_os << setw(4) << str << ' ';
						else     a_os << "     ";
					}
					a_os << "|\n";
				}
				a_os << '|' << string (5*COLS, '_') << "|\n";
				a_os << '\n';
			}
			return a_os;
		} // show_tables

	//------
	private:
	//------

		static bool   s_nibbles;
		static size_t s_prefill;
		static size_t s_mask;

		vector<State> m_table; ///< State tables

	}; // class Table
	bool   Table::s_nibbles{false};
	size_t Table::s_prefill{1};
	size_t Table::s_mask{0xffULL};

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	//__________________________________________________________________________
	/// @brief GreasedGrep implements overall state-transition operations
	class
	GreasedGrep : public Table
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
			while (--a_argc)
			{
				ingest (*++a_argv);
			}
		} // ctor

		//----------------------------------------------------------------------
		/// @brief ftor
		void
		operator() ()
		//----------------------------------------------------------------------
		{
			// Validate ingested args
			if (m_accept.size () < 2 && m_reject.size () < 2)
			{
				synopsis ("specify at least one accept or reject str");
			}

			// Check for valid directory
			if (!fs::is_directory (m_directory))
			{
				synopsis ("last arg must be dir");
			}

			// Compile and check for collisions between accept and reject lists
			compile ();

			// Initialize firsts to enable buffer skipping
			for (size_t i=0; i<256; ++i)
			{
				const Atom& element (m_state1[i]);
				auto t{element.tgt ()};
				if (t) m_firsts += static_cast<u08_t>(i);
			}

			// Visually inspect planes
			if (m_debug)
			{
				show_tables (cout);
				show_tokens (cout);
				cout << "\tFIRSTS: " << m_firsts << endl;
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
			// Handle options
			if (a_str.size () == 2 && a_str[0] == '-')
			{
				if (m_accept.size () > 1 || m_reject.size () > 1)
				{
					synopsis ("command-line options must precede other args");
				}
				switch (a_str[1])
				{
					case 'c': m_caseless = false; return;
					case 'd': m_debug    =     1; return;
					case 'n': m_nibbles  =  true; return;
					case 's': m_suppress =  true; return;
				}
				synopsis (a_str.data ());
			}
			State::nibbles (m_nibbles);
			Table::nibbles (m_nibbles);

			if (m_directory.size ())
			{
				string_view candidate {m_directory};

				char c0               {candidate[0]};
				bool rejecting        {c0 == '-'};
				auto& field           {rejecting ? m_reject : m_accept};

				candidate.remove_prefix ((c0=='+' || c0=='-') ? 1 : 0);

				if (candidate.size () < 2)
				{
					synopsis ("pattern strings must be longer than 1 byte");
				}
				field.push_back (candidate);
#if !GG_COMPILE
				// This compile passes
				i24_t sign            {rejecting?-1:+1};
				compile (sign, candidate);
#endif
			}
			m_directory = a_str;
		} // ingest

		//----------------------------------------------------------------------
		/// @brief ingest inserts state-transition table data
		void
		compile (int a_sign=0)
		//----------------------------------------------------------------------
		{
			if (!a_sign)
			{
				compile (+1);
				compile (-1);
				return;
			}
			bool rejecting {a_sign == -1};
			auto& field    {rejecting ? m_reject : m_accept};
			size_t I       {field.size ()};

			//< m_noreject optimizes inner loop
			if (rejecting && I > 1)
			{
				m_noreject = false;
			}

#if GG_COMPILE
				// This compile fails
			for (size_t i = 1; i < I; ++i)
			{
				compile (a_sign, field[i]);
			}
#endif
		}

		//----------------------------------------------------------------------
		/// @brief insert either case-sensitive or both case letters into tree
		///
		/// Distribute characters into state tables for searching.
		/// TODO nibbles handling is under development and may not work yet.
		void
		insert (char* a_chars, auto& a_from, auto& a_next, bool a_nibbles=false)
		//----------------------------------------------------------------------
		{
			if (m_debug)
			{
				printf ("\tINSERT %2.2x and %2.2x\n", a_chars[0], a_chars[1]);
			}
			auto c0{a_chars[0]};
			auto c1{a_chars[1]};
			// TODO validate nibbles insertion
			if (a_nibbles && m_nibbles)
			{
				auto upper00{ c0     & 0x0f};
				auto upper01{(c0>>4) & 0x0f};
				auto lower10{ c1     & 0x0f};
				auto lower11{(c1>>4) & 0x0f};
				char hi[2], lo[2];
				hi[0] = upper01;
				hi[1] = lower11;
				lo[0] = upper00;
				lo[1] = lower10;
				insert (hi, a_from, a_next, false);
				insert (lo, a_from, a_next, false);
				return;
			}
			else
			{
				Atom& element{operator[] (a_from)[c0]};
				auto to{element.tgt ()};
				a_next = a_from;
				if (to) {
					a_from = to;
				}
				else
				{
					a_from = Table::size ();
					operator++ ();
				}
				element.tgt (a_from);
				if (c0 != c1)
				{
					operator[] (a_next)[c1].tgt (a_from);
				}
			}
		} // insert

		//----------------------------------------------------------------------
		/// @brief compile a single string argument
		///
		/// Distribute characters into state tables for searching.
		void
		compile (int a_sign, string_view a_str)
		//----------------------------------------------------------------------
		{
			if (m_debug)
			{
				printf ("\tCOMPILE %+d: %s\n", a_sign, a_str.data ());
			}
			bool rejecting        {a_sign == -1};
			auto from             {m_root};
			auto next             {from};
			char last[2]          {0,0};
			auto& field           {rejecting ? m_reject : m_accept};
			i24_t id              {a_sign*static_cast<i24_t> (field.size ())};

			// Initially, the string as given is searched
			// TODO generate variations like soundex/levenshtein, fatfinger
			// TODO handle collision for variations
			// e.g. "than" and "then" are legitimate mutual variations.
			vector<string> strs;
			strs.emplace_back (string(a_str));

			// Insert variations into transition tree
			for (auto& str: strs)
			{

				// Insert a_str into state transition tree
				for (char u:str)
				{
					if (m_caseless)
					{
						last[0] = static_cast<char> (toupper (u));
						last[1] = static_cast<char> (tolower (u));
						insert (last, from, next, m_nibbles);
					}
					else
					{
						last[0] = last[1] = u;
						insert (last, from, next, m_nibbles);
					}
				}
				if (m_caseless)
				{
					for (auto c:last) operator[] (next)[c].str (id);
				}
				else
				{
					operator[] (next)[last[0]].str (id);
				}
			}
		}

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
				auto tgt{1}; // State
				auto str{0}; // 
				// inner loop (Finite State Machine optimization)
				for (auto c: contents)
				{
					auto n00{c};
					if (m_nibbles)
					{
						// Two-step for nibbles
						n00 = c & 0xf;
						auto element{operator[] (tgt)[(c>>4) & 0x0f]};
						if (!element.tgt ()) break;
					}
					auto element = operator[] (tgt)[n00];
					tgt = element.tgt ();
					str = element.str ();
					if (str) {
						if (str < 0) return; ///< Immediate rejection
						// If not immediate rejection, add to rejected list
						auto& chose{(str>0)?accepted:rejected};
						chose.insert (str);
						bool full_accept{m_accept.size () == accepted.size ()};
						// completion optimization
						done = (m_noreject && full_accept);
					}
					if (done || !tgt) break;
				}
				contents.remove_prefix (1);
				begin = contents.find_first_of (m_firsts);
			}

			// Report files having all accepteds and no rejecteds.
			if (!rejected.size () && accepted.size () == m_accept.size ())
			{
				auto report{fmt::format ("{}\n", a_filename)};
				// Using the unix write primitive guarantees atomicity
				// This is needed to avoid thread contention
				auto wrote = write (1, report.c_str (), report.size ());
				// This next line should never be executed.
				if (wrote == -1) printf ("%s\n", a_filename);
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
							// mapped_search should be launched as a thread
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

		//----------------------------------------------------------------------
		void
		show_tokens (ostream& a_os)
		//----------------------------------------------------------------------
		{
			a_os << "\tACCEPT:" << endl;
			for (size_t i=1; i < m_accept.size (); ++i)
			{
				a_os << setw (2) << i << ": " << m_accept[i] << endl;
			}
			a_os << "\tREJECT:" << endl;
			for (size_t i=1; i < m_reject.size (); ++i)
			{
				a_os << setw (2) << i << ": " << m_reject[i] << endl;
			}
		} // show_tokens

		//dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
		vector<string_view> m_accept {{""}}; ///< list of accept {str} args
		vector<string_view> m_reject {{""}}; ///< list of reject {str} args
		u08_t m_root{1};                     ///< syntax tree root plane number
		size_t m_debug{0};                   ///< turns on verbosity
		bool m_noreject{true};               ///< are there reject strings?
		bool m_suppress{false};              ///< suppress error messages
		bool m_caseless{true};               ///< turn on case insensitivity
		bool m_nibbles {false};              ///< use small-plane nibble code

		string_view m_directory;
		string m_firsts;                     ///< string of {arg} first letters
		State& m_state1{operator[] (m_root)};///< root state plane
	}; // class GreasedGrep

} // namespace Lettvin
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

//------------------------------------------------------------------------------
/// @brief main (program execution entrypoint)
int
main (int a_argc, char** a_argv)
//------------------------------------------------------------------------------
{
	std::ios::sync_with_stdio (true);
	if (a_argc < 3)
	{
		Lettvin::synopsis ("At least one pattern and a directory required.");
	}
	Lettvin::GreasedGrep gg (a_argc, a_argv);
	gg ();
	return 0;
} // main
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

