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

static const char* s_path=".";
static const char* s_synopsis =
R"Synopsis(Greased Grep version %u.%u.%u

PATH: %s

USAGE: gg [-c] [-n] [-s] [-t] [-v] [+|-]{str} [[+|-]{str}...] {path} 

Greased Grep UTF8 search for files having (case insensitive):
    all instances of +{str} or {str} and
	no  instances of -{str} instances in
	files found along {path}

        {str} are simple strings (no regex).
        {str} may be single-quoted to avoid shell interpretation.

ARGUMENTS:

    [+]{str}        # add accept string (+ optional)

    -{str}          # add reject string

    {path}          # file or top directory for recursive search

ARGUMENT OPTIONS: (TODO)

    When the --variant option is used
    A {str} followed by a bracket-list triggers variant insertion
    Examples:
       $ gg -v copyright[acronym,c,f,soundex,nyssis] .
    Available:
       a or acronym           to insert variants like M.I.T.
       c or contraction       to insert variants like MIT
       f or fatfinger         to insert variants like NUR
       l or levenshtein1      to insert variants like MTI
       m or metaphone
       n or nyssis
       s or soundex
       t or thesaurus         to insert synonyms
       u or unicode           to insert NFKD variants

OPTIONS:

    -c, --caseless  # case sensitive search

    -d, --debug     # turn on debugging output

    -n, --nibbles   # use nibbles (lower memory use half-speed search)

    -s, --suppress  # suppress permission denied errors

    -t, --test      # test algorithms (unit and timing)  TODO

    -v, --variant # enable variant syntax with [] brackets

OUTPUT:

    canonical paths of files fulfilling the set conditions.

EXAMPLES:

    $ gg include /usr/local/src
        # find all files having the string 'inlude' in /usr/local/src

    $ gg '#include <experimental/filesystem>' /usr/local/src
        # find all files having the quoted string in /usr/local/src

    $ gg copyright -Lettvin .
        # Find all files with missing or other than Lettvin copyright.

    $ gg 愚公移山 .
        # Find the foolish old man who moved the mountains
)Synopsis";

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
// TODO measure performance against fgrep/ack/ag
//      publishing performance will make gg more attractive
// TODO ingest args with ctor but compile strs at beginning of ftor
//      compilation in ftor currently fails
// TODO use memcmp for unique final string
//      When the tail end of a search is unique memcmp is faster
// TODO run searches in threads for higher performance
//      thread-safe Table and atomic output: concurrent processing is possible
// TODO implement self-test (-t)
//      client-usable as opposed to unit-test and performance test
// TODO translate UTF8->UnicodeCodepoint->NFKD->UnicodeCodepoint->UTF8
//      strings with identical appearance should be comparable
//      this could be done by decomposing and recomposing during compilation
//      for instance; convert to Unicode Codepoints, and decompose, then
//      recompose to canonical NFKD, then reconvert to UTF8, then
//      strings so recomposed can be compared properly
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
#include <sstream>                 // string_stream
#include <iomanip>                 // setw and other cout formatting
#include <thread>

//..............................................................................
#include <string>                  // container
#include <vector>                  // container
#include <map>                     // container
#include <set>                     // container

//..............................................................................
#include "catch.hpp"               // Testing framework

//..............................................................................
#include "gg_version.h"            // version
#include "gg.h"                    // declarations
#include "thread_queue.h"          // filename distribution to threads

namespace fs = std::experimental::filesystem;
using namespace std;  // No naming collisions in this small namespace

//------------------------------------------------------------------------------
/// @brief synopsis (document usage in case of failure)
void Lettvin::synopsis (const char* a_message, ...)
//------------------------------------------------------------------------------
{
	if (a_message != nullptr)
	{
		printf (" # ERROR: ");
	}
	va_list args;
	va_start (args, a_message);
	vprintf (a_message, args);
	va_end (args);
	printf ("\n");
	printf (s_synopsis,
			s_version.major,
			s_version.minor,
			s_version.build,
			s_path);
	exit (1);
} // synopsis

//------------------------------------------------------------------------------
/// @brief synopsis (document usage in case of failure)
void Lettvin::syntax (const char* a_message, ...)
//------------------------------------------------------------------------------
{
	printf (" # SYNTAX ERROR: ");
	va_list args;
	va_start (args, a_message);
	vprintf (a_message, args);
	va_end (args);
	printf ("\n");
	printf (s_synopsis,
			s_version.major,
			s_version.minor,
			s_version.build,
			s_path);
	exit (1);
}

//------------------------------------------------------------------------------
/// @brief nibbles converts algorithm from byte to nibble tables.
/// This saves considerable memory but slows the search by a factor of 2.
void Lettvin::nibbles ()
//------------------------------------------------------------------------------
{
	s_nibbles = true;
	s_prefill = 2;
	s_mask    = 0x0fULL;
	s_size    = 16;
} // nibbles

//------------------------------------------------------------------------------
int
Lettvin::debugf (size_t a_debug, const char *fmt, ...)
//------------------------------------------------------------------------------
{
	static const char* indent{"  "};
	int ret = 0;
	if (s_debug >= a_debug)
	{
		va_list args;
		va_start (args, fmt);
		printf (" #'");
		for (size_t i=0; i < a_debug; ++i)
		{
			printf ("%s", indent);
		}
		printf ("DBG(%lu): ", a_debug);
		ret = vprintf (fmt, args);
		va_end (args);
	}
	return ret;
} // debugf

//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

//------------------------------------------------------------------------------
Lettvin::Table::
Table ()
//------------------------------------------------------------------------------
{
	m_table.reserve (s_prefill*256);
	for (size_t i=0; i < s_prefill; ++i)
	{
		operator++ ();
		operator++ ();
	}
} // ctor

//------------------------------------------------------------------------------
Lettvin::State&
Lettvin::Table::
operator[] (u08_t a_offset)
//------------------------------------------------------------------------------
{
	return m_table[a_offset & s_mask];
} // operator[]

//------------------------------------------------------------------------------
void
Lettvin::Table::
operator++ ()
//------------------------------------------------------------------------------
{
	m_table.resize (m_table.size () + 1);
} // operator++

//------------------------------------------------------------------------------
size_t
Lettvin::Table::
size ()
//------------------------------------------------------------------------------
{
	return m_table.size ();
} // size

//------------------------------------------------------------------------------
std::ostream&
Lettvin::Table::
show_tables (ostream& a_os)
//------------------------------------------------------------------------------
{
	size_t COLS{s_nibbles ? 4ULL : 16ULL};
	size_t ROWS{s_nibbles ? 4ULL : 16ULL};
	for (size_t state=0; state < m_table.size (); ++state)
	{
		auto& plane{m_table[state]};
		a_os << " # " << endl << " # ";
		for (unsigned col=0; col < COLS; ++col)
		{
			printf ("   %2.2x", col);
		}
		a_os << endl << " #  " << string (5*COLS, '_') << 
			"     PLANE: " << state << endl << " # ";
		for (unsigned row=0; row < ROWS*COLS; row+=COLS)
		{
			a_os << '|';
			for (unsigned col=0; col < COLS; ++col)
			{
				char id{static_cast<char>(row+col)};
				Atom& entry{plane[id]};
				int tgt{static_cast<int>(entry.tgt ())};
				char gra{id>=' '&&id<='~'?id:'?'};
				if (tgt) a_os << gra << setw (3) << tgt << ' ';
				else     a_os << ".....";
			}
			printf ("|\n # |");
			for (size_t col=0; col < COLS; ++col)
			{
				Atom& entry{plane[row+col]};
				int str{static_cast<int>(entry.str ())};
				if (str) a_os << setw (4) << str << ' ';
				else     a_os << ".....";
			}
			printf ("|%2.2x\n # ", row);
		}
		a_os << "|" << string (5*COLS, '_') << "|" << endl;
	}
	return a_os;
} // show_tables

//------------------------------------------------------------------------------
/// @brief insert either case-sensitive or both case letters into tree
///
/// Distribute characters into state tables for searching.
void
Lettvin::Table::
insert (
		char* a_chars,
		auto& a_from,
		auto& a_next,
		bool a_stop,
		bool a_nibbles)
//------------------------------------------------------------------------------
{
	auto c0{a_chars[0]};
	auto c1{a_chars[1]};
	if (a_nibbles && s_nibbles)
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
		insert (hi, a_from, a_next, a_stop, false);
		insert (lo, a_from, a_next, a_stop, false);
		return;
	}
	else
	{
		debugf (1, "INSERT %2.2x and %2.2x on plane %x\n",
				a_chars[0], a_chars[1], a_from);
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

//------------------------------------------------------------------------------
/// @brief insert strings into tables
void
Lettvin::Table::
insert (string_view a_str, i24_t id)
{
	auto from      {s_root};
	auto next      {from};
	char last[2]   {0,0};

	// Insert initial char of str for skipping.
	if (s_caseless)
	{
		s_firsts += static_cast<char> (toupper (a_str[0]));
		s_firsts += static_cast<char> (tolower (a_str[0]));
	}
	else
	{
		s_firsts += a_str[0];
	}

	// Insert a_str into state transition tree
	for (size_t I=a_str.size () - 1, i=0; i <= I; ++i)
		//for (char u:str)
	{
		char u{a_str[i]};
		bool stop{i==I};
		if (s_caseless)
		{
			last[0] = static_cast<char> (toupper (u));
			last[1] = static_cast<char> (tolower (u));
			insert (last, from, next, stop, s_nibbles);
		}
		else
		{
			last[0] = last[1] = u;
			insert (last, from, next, stop, s_nibbles);
		}
	}
	if (s_caseless)
	{
		for (auto c:last) operator[] (next)[c&s_mask].str (id);
	}
	else
	{
		debugf (1, "LINK %x %lx %x\n", next, last[0]&s_mask, id);
		operator[] (next)[last[0]&s_mask].str (id);
	}
}

//------------------------------------------------------------------------------
/// @brief find and report found strings
///
/// when reject list is empty, terminate on completion of accept list
/// when reject list is non-empty, terminate on first reject
void
Lettvin::Table::
search (void* a_pointer, auto a_bytecount, const char* a_label)
//------------------------------------------------------------------------------
{
	//debugf (1, "SEARCH %s\n", a_label);
	set<i24_t> accepted  {0};
	set<i24_t> rejected  {};
	string_view contents (static_cast<char*> (a_pointer), a_bytecount);
	size_t begin = contents.find_first_of (s_firsts);
	bool done{false};
	// outer loop (skip optimization)
	while (begin != string_view::npos && !done)
	{
		//debugf (1, "ANCHOR\n");
		contents.remove_prefix (begin);
		auto tgt{1}; // State
		auto str{0}; // 
		// inner loop (Finite State Machine optimization)
		for (char c: contents)
		{
			//debugf (1, "OFFSET\n");
			auto n00{c};
			if (s_nibbles)
			{
				// Two-step for nibbles
				n00 = (c>>4) & 0xf;
				//debugf (1, "NIBBLE H %2.2x %2.2x\n", n00, tgt);
				auto element{operator[] (tgt)[n00]};
				tgt = element.tgt ();
				n00 = c & 0xf;
				//debugf (1, "NIBBLE L %2.2x %2.2x\n", n00, tgt);
				if (!tgt) break;
			}
			else
			{
				//debugf (1, "BYTE %c\n", c);
			}
			auto element = operator[] (tgt)[n00];
			tgt = element.tgt ();
			str = element.str ();
			if (str) {
				if (str < 0) return; ///< Immediate rejection
				// If not immediate rejection, add to rejected list
				auto& chose{(str>0)?accepted:rejected};
				chose.insert (str);
				bool full_accept{s_accept.size () == accepted.size ()};
				// completion optimization
				done = (s_noreject && full_accept);
			}
			if (done || !tgt) break;
		}
		contents.remove_prefix (1);
		begin = contents.find_first_of (s_firsts);
	}

	// Report files having all accepteds and no rejecteds.
	if (!rejected.size () && accepted.size () == s_accept.size ())
	{
		auto report{fmt::format ("{}\n", a_label)};
		// Using the unix write primitive guarantees atomicity
		// This is needed to avoid thread contention
		auto wrote = write (1, report.c_str (), report.size ());
		// This next line should never be executed.
		if (wrote == -1) printf ("%s\n", a_label);
	}
} // search

//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

//------------------------------------------------------------------------------
/// @brief ctor
Lettvin::GreasedGrep::
GreasedGrep (int a_argc, char** a_argv) // ctor
//------------------------------------------------------------------------------
{
	while (--a_argc)
	{
		ingest (*++a_argv);
	}
} // ctor

//------------------------------------------------------------------------------
/// @brief ftor
void
Lettvin::GreasedGrep::
operator ()()
//------------------------------------------------------------------------------
{
	// Run unit and timing tests.
	if (s_test)
	{
		printf ("TESTS\n");
		double minimized{1.0};
		static const size_t overhead_loop_count{10};
		for (size_t i=overhead_loop_count; i; --i)
		{
			minimized = min (minimized, interval (noop));
		}
		s_overhead = minimized;
		cout << " # overhead min: " << s_overhead << endl;
		return;
	}

	// Validate sufficient args.
	if (s_target.size () < 1 &&
		((s_accept.size () < 2) && (s_reject.size () < 2)))
	{
		Lettvin::synopsis ("pattern(s) and directory required.");
	}

	// Validate ingested args
	if (s_accept.size () < 2 && s_reject.size () < 2)
	{
		synopsis ("specify at least one accept or reject str");
	}

	// Check for valid directory
	if (!fs::is_directory (s_target) &&
		!fs::is_regular_file (s_target))
	{
		synopsis ("last arg must be dir or file");
	}

	// Compile and check for collisions between accept and reject lists
	compile ();

	// Initialize firsts to enable buffer skipping
	debugf (1, "FIRSTS B: '%s'\n", s_firsts.c_str ());
	sort (s_firsts.begin (), s_firsts.end ());
	auto last = unique (s_firsts.begin (), s_firsts.end ());
	s_firsts.erase (last, s_firsts.end ());
	debugf (1, "FIRSTS A: '%s'\n", s_firsts.c_str ());

	// Visually inspect planes
	if (s_debug)
	{
		show_tables (cout);
		show_tokens (cout);
	}
	debugf (1, "FIRSTS: %s\n", s_firsts.c_str ());

	// Find files and search contents
	if (fs::is_directory (s_target))
	{
		walk (s_target);
	}
	else
	{
		mapped_search (s_target.data ());
	}

} // operator ()

//------------------------------------------------------------------------------
/// @brief ingest inserts state-transition table data
bool
Lettvin::GreasedGrep::
option (string_view a_str)
//------------------------------------------------------------------------------
{
	size_t size {a_str.size ()};
	bool two    {size == 2};
	bool more   {size >  2};
	bool minus1 {a_str[0] == '-'};
	bool opt    {two && minus1};
	bool bad    {!(two || more)};
	bool strs   {s_accept.size () > 1 || s_reject.size () > 1};
	bool nbls   {s_nibbles};
	char letter {two ? a_str[1] : '\0'};

	if (!minus1)
	{
		return false;
	}

	if (strs)
	{
		synopsis ("(%s) options must precede other args", a_str.data ());
	}
	if (bad)
	{
		synopsis ("command-line options must be two or more chars");
	}

	if      (a_str == "--caseless" || (opt && letter == 'c')) s_caseless = true;
	else if (a_str == "--debug"    || (opt && letter == 'd')) s_debug   += 1;
	else if (a_str == "--nibbles"  || (opt && letter == 'n')) s_nibbles  = true;
	else if (a_str == "--suppress" || (opt && letter == 's')) s_suppress = true;
	else if (a_str == "--test"     || (opt && letter == 't')) s_test     = true;
	else if (a_str == "--variant"  || (opt && letter == 'v')) s_variant  = true;
	else if (opt)
	{
		synopsis ("unknown arg");
	}
	else
	{
		return false;
	}
	if (s_nibbles && !nbls)
	{
		nibbles ();
	}
	return true;
}

//------------------------------------------------------------------------------
/// @brief ingest inserts state-transition table data
void
Lettvin::GreasedGrep::
ingest (string_view a_str)
//------------------------------------------------------------------------------
{
	// Handle options
	size_t size{a_str.size ()};
	bool minus1{size == 2 && a_str[0] == '-'};
	bool minus2{size >= 3 && a_str[0] == '-' && a_str[1] == '-'};

	if ((minus1 || minus2) && option (a_str)) return;

	if (s_target.size ())
	{
		string_view candidate {s_target};

		char c0               {candidate[0]};
		bool rejecting        {c0 == '-'};
		auto& field           {rejecting ? s_reject : s_accept};

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
	s_target = a_str;
} // ingest

//------------------------------------------------------------------------------
/// @brief ingest inserts state-transition table data
void
Lettvin::GreasedGrep::
compile (int a_sign)
//------------------------------------------------------------------------------
{
	if (!a_sign)
	{
		compile (+1);
		compile (-1);
		return;
	}
	bool rejecting {a_sign == -1};
	auto& field    {rejecting ? s_reject : s_accept};
	size_t I       {field.size ()};

	//< s_noreject optimizes inner loop
	if (rejecting && I > 1)
	{
		s_noreject = false;
	}

#if GG_COMPILE
		// This compile fails
	for (size_t i = 1; i < I; ++i)
	{
		compile (a_sign, field[i]);
	}
#endif
}

//------------------------------------------------------------------------------
/// @brief compile a single string argument
///
/// Distribute characters into state tables for searching.
void
Lettvin::GreasedGrep::
compile (int a_sign, string_view a_sv)
//------------------------------------------------------------------------------
{
	debugf (1, "COMPILE %+d: %s\n", a_sign, a_sv.data ());
	bool rejecting   {a_sign == -1};
	auto from        {s_root};
	auto next        {from};
	char last[2]     {0,0};
	auto& field      {rejecting ? s_reject : s_accept};
	i24_t id         {a_sign*static_cast<i24_t> (field.size () - 1)};
	string b_str     {};
	string a_str     {a_sv};

	vector<string> variant_names;

	if (s_variant)
	{
		map<string, size_t>::const_iterator citer;
		size_t brace_init{a_sv.find_first_of ('[')};
		if (brace_init != string_view::npos)
		{
			size_t brace_fini{a_sv.find_first_of (']', brace_init + 1)};
			if (brace_fini == string_view::npos)
			{
				syntax ("braced variants");
			}
			debugf (1, "BR %zu %zu\n", brace_init, brace_fini);
			b_str = a_sv.substr (brace_init + 1, brace_fini - brace_init - 1);
			a_str = a_sv.substr (0, brace_init);
			debugf (1, "Braced [%s][%s]\n", a_str.c_str (), b_str.c_str ());
			size_t comma = b_str.find_first_of (',');
			while (comma != string::npos)
			{
				string token = b_str.substr (0, comma);
				b_str = b_str.substr (comma + 1);
				comma = b_str.find_first_of (',');
				variant_names.push_back (token);
				citer = s_variants.find (token);
				if (citer == s_variants.end ())
				{
					syntax ("BAD variant name [%s]\n", token.c_str ());
				}
				debugf (1, "variant: %s\n", token.c_str ());
			}
			if (b_str.size ())
			{
				variant_names.push_back (b_str);
				citer = s_variants.find (b_str);
				if (citer == s_variants.end ())
				{
					syntax ("BAD variant name [%s]\n", b_str.c_str ());
				}
				debugf (1, "variant: %s\n", b_str.c_str ());
			}
		}
	}

	// Initially, the string as given is searched
	// TODO generate variants like soundex/levenshtein, fatfinger
	// TODO handle collision for variants
	// TODO generate all NFKD variants for insertion
	// e.g. "than" and "then" are legitimate mutual variants.
	vector<string> strs;
	strs.emplace_back (a_str);

	for (auto& variant_name:variant_names)
	{
		// TODO for each variant_name, generate and strs.emplace_back (variant).
		// Multiple variants are typical for each variant_name
	}

	// Insert variants into transition tree
	for (auto& str: strs)
	{
		insert (str, id);
	}
}

//------------------------------------------------------------------------------
/// @brief map file into memory and call search
///
/// https://techoverflow.net/2013/08/21/a-simple-mmap-readonly-example/
void
Lettvin::GreasedGrep::
mapped_search (const char* a_filename)
//------------------------------------------------------------------------------
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

		if (contents == MAP_FAILED)
		{
			debugf (1, "MAP FAILED: %s\n", a_filename);
		}
		else
		{
			search (contents, filesize, a_filename);
			int rc = munmap (contents, filesize);
			if (rc != 0) synopsis ("munmap failed");
		}

		close (fd);
	}
	else
	{
		debugf (1, "OPEN FAILED: %s\n", a_filename);
	}
} // mapped_search

//------------------------------------------------------------------------------
/// @brief walk organizes search for strings in memory-mapped file
void
Lettvin::GreasedGrep::
walk (const fs::path& a_path)
//------------------------------------------------------------------------------
{
	try
	{
#if true
		// Account for existing main thread
		unsigned int cpus{thread::hardware_concurrency ()};
		vector<thread> threads;
		Lettvin::ThreadedQueue<string> tq (cpus + 1);  // 1 ahead of workers
		// Note worker threads are 1 less than cpus
		for (unsigned int thid = 1; thid < cpus; thid++)
		{
			threads.emplace_back (
				thread{
					[&] ()
					{
						string filename;
						while ((filename = tq.pop ()).size () != 0)
						{
							mapped_search (filename.c_str ());
						}
					}
				}
			);
		}
		// Master thread
		threads.emplace_back (
			thread{
				[&] ()
				{
					string filename;
					for (auto& item:
						fs::recursive_directory_iterator (a_path))
					{
						fs::path path{fs::canonical (fs::path (item.path ()))};
						const char* filename{path.c_str ()};
						if (fs::is_regular_file (item.status ()))
						{
							try
							{
								tq.push (path);
							}
							catch (...)
							{
								if (!s_suppress)
								{
									printf (
										"gg: %s file Permission denied\n",
										filename);
								}
							}
						}
					}
					for (size_t cpu=1; cpu < cpus; ++cpu)
					{
						tq.push ("");
					}
				}
			}
		);
		for (auto& thrd:threads) thrd.join ();
#else
		for (auto& item: fs::recursive_directory_iterator (a_path))
		{
			fs::path path{fs::canonical (fs::path (item.path ()))};
			const char* filename{path.c_str ()};
			if (fs::is_regular_file (item.status ()))
			{
				try
				{
					mapped_search (filename);
				}
				catch (...)
				{
					if (!s_suppress)
					{
						printf ("gg: %s file Permission denied\n",
								filename);
					}
				}
			}
		}
#endif
	}
	catch (...)
	{
		if (!s_suppress)
		{
			printf ("gg: %s dir Permission denied\n",
					a_path.filename ().c_str ());
		}
	}
} // walk

//------------------------------------------------------------------------------
void
Lettvin::GreasedGrep::
show_tokens (ostream& a_os)
//------------------------------------------------------------------------------
{
	a_os << " # ACCEPT:" << endl;
	for (size_t i=1; i < s_accept.size (); ++i)
	{
		a_os << " # " << setw (2) << i << ": " << s_accept[i] << endl;
	}
	a_os << " # REJECT:" << endl;
	for (size_t i=1; i < s_reject.size (); ++i)
	{
		a_os << " # " << setw (2) << i << ": " << s_reject[i] << endl;
	}
} // show_tokens


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#ifndef GG_TEST
//------------------------------------------------------------------------------
/// @brief main (program execution entrypoint)
int
main (int a_argc, char** a_argv)
//------------------------------------------------------------------------------
{
	try
	{
		std::ios::sync_with_stdio (true);
		fs::path app{fs::canonical (fs::path (a_argv[0]))};
		s_path = const_cast<const char*>(app.c_str ());
		Lettvin::GreasedGrep gg (a_argc, a_argv);
		gg ();
	}
	catch (...)
	{
		Lettvin::synopsis ("Unexpected signal caught.");
	}
	return 0;
} // main
#endif
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

