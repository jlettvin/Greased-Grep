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
#include <regex>

//..............................................................................
#include <string>                  // container
#include <vector>                  // container
#include <map>                     // container
#include <set>                     // container

//..............................................................................
#include "catch.hpp"               // Testing framework

//..............................................................................
#include "gg_version.h"            // s_version and s_synopsis
#include "gg.h"                    // declarations
#include "state_table.h"           // Mechanism for finite state machine
#include "thread_queue.h"          // filename distribution to threads
#include "utility.h"               // tokenize

//..............................................................................
#include "variant.h"               // variant implementations

namespace fs = std::experimental::filesystem;
using namespace std;  // No naming collisions in this small namespace

// Stroustrup recommends returning multiple like this.
tuple <string, int> experiment1_source ()
{
	return tuple<string, int> ("hello", 1);
}

void experiment1_target ()
{
	auto [hello, code] = experiment1_source ();
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
	int32_t ret = 0;
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

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

//------------------------------------------------------------------------------
Lettvin::Atom::
Atom ()
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
Lettvin::integral_t
Lettvin::Atom::
integral () const
{
	return m_the.integral;
} // integral

//------------------------------------------------------------------------------
uint8_t
Lettvin::Atom::
tgt () const
//------------------------------------------------------------------------------
{
	return m_the.state.tgt;
} // tgt

//------------------------------------------------------------------------------
Lettvin::i24_t
Lettvin::Atom::
str () const
//------------------------------------------------------------------------------
{
	return m_the.state.str;
} // str

//------------------------------------------------------------------------------
void
Lettvin::Atom::
tgt (uint8_t a_tgt)
//------------------------------------------------------------------------------
{
	m_the.state.tgt = a_tgt;
} // tgt

//------------------------------------------------------------------------------
void
Lettvin::Atom::
str (i24_t a_str)
//------------------------------------------------------------------------------
{
	m_the.state.str = a_str;
} // str

//SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS

//------------------------------------------------------------------------------
Lettvin::State::
State ()
//------------------------------------------------------------------------------
	: m_handle (s_size)
{
} // State

//------------------------------------------------------------------------------
Lettvin::Atom&
Lettvin::State::
operator[] (uint8_t a_off)
//------------------------------------------------------------------------------
{
	return m_handle[a_off];
} ///< operator[]

//------------------------------------------------------------------------------
vector<Lettvin::Atom>&
Lettvin::State::
handle ()
//------------------------------------------------------------------------------
{
	return m_handle;
}

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

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
operator[] (uint8_t a_offset)
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
			bool content{false};
			for (unsigned col=0; col < COLS; ++col)
			{
				Atom& entry{plane[static_cast<char>(row+col)]};
				int32_t tgt{static_cast<int32_t>(entry.tgt ())};
				int32_t str{static_cast<int32_t>(entry.str ())};
				content |= !!tgt;
				content |= !!str;
			}
			if (!content)
			{
				continue;
			}
			a_os << '|';

			for (unsigned col=0; col < COLS; ++col)
			{
				char id{static_cast<char>(row+col)};
				Atom& entry{plane[id]};
				int32_t tgt{static_cast<int32_t>(entry.tgt ())};
				char gra{id>=' '&&id<='~'?id:'?'};
				if (tgt) a_os << gra << setw (3) << tgt << ' ';
				else     a_os << ".....";
			}
			printf ("|\n # |");
			for (size_t col=0; col < COLS; ++col)
			{
				Atom& entry{plane[row+col]};
				int32_t str{static_cast<int32_t>(entry.str ())};
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
insert (string_view a_str, i24_t id, size_t setindex)
{
	auto from      {s_root};
	auto next      {from};
	char last[2]   {0,0};

	// Cardinal setindex retrieves this terminal set
	set<int32_t>& setitem{s_set[setindex]};

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
#if SETINDIRECT
		setitem.insert (id);
		for (auto c:last) operator[] (next)[c&s_mask].str (setindex);
#else
		for (auto c:last) operator[] (next)[c&s_mask].str (id);
#endif
	}
	else
	{
		debugf (1, "LINK %x %lx %x\n", next, last[0]&s_mask, id);
#if SETINDIRECT
		setitem.insert (id);
		operator[] (next)[last[0]&s_mask].str (setindex);
#else
		operator[] (next)[last[0]&s_mask].str (id);
#endif
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
#if SETINDIRECT
				set<int32_t>& setitem{s_set[str]};
				for (auto item:setitem)
				{
					if (item < 0) return; ///< Immediate rejection
					// If not immediate rejection, add to rejected list
					auto& chose{(item>0)?accepted:rejected};
					chose.insert (item);
					bool full_accept{s_accept.size () == accepted.size ()};
					// completion optimization
					done = (s_noreject && full_accept);
					if (done) break;
				}
#else
				if (str < 0) return; ///< Immediate rejection
				// If not immediate rejection, add to rejected list
				auto& chose{(str>0)?accepted:rejected};
				chose.insert (str);
				bool full_accept{s_accept.size () == accepted.size ()};
				// completion optimization
				done = (s_noreject && full_accept);
#endif
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

//------------------------------------------------------------------------------
// @brief dump tree to file
//
// TODO must output state size and table size
// TODO best practice read entire, then swap if needed
// TODO output s_order.u64 to file to establish file order
void Lettvin::Table::dump (const char* a_filename, const char* a_title)
{
	int32_t fd = open (
			a_filename,
			O_RDWR|O_CREAT,
			S_IRWXU|S_IRWXG|S_IRWXO);
	if (fd > 0)
	{
		debugf (1, "dump PASS: %s: %s\n", a_filename, a_title);
		char   zero{0};
		size_t endian{0x3736353433323130};
		const char* cdcz{"\x04\x1a"};
		size_t consumed{34+strlen (a_title)};
		size_t needed{256-consumed};

		write (fd, "gg dump:", 8);
		write (fd, "endian: ", 8);
		write (fd, &endian   , 8);
		write (fd, " title: ", 8);
		write (fd, a_title   , strlen (a_title));
		write (fd, cdcz      , 2);
		for (size_t i=0; i<needed; ++i)
		{
			write (fd, &zero, 1);
		}
		
		for (auto& state:m_table)
		{
			for (auto& atom: state.handle ())
			{
				union { unsigned integral; uint8_t u08[4]; } datum{
					.integral = atom.integral ()};
				write (fd, &datum.u08[s_order.u08.array[0]], 1);
				write (fd, &datum.u08[s_order.u08.array[1]], 1);
				write (fd, &datum.u08[s_order.u08.array[2]], 1);
				write (fd, &datum.u08[s_order.u08.array[3]], 1);
			}
		}
	}
	else
	{
		debugf (1, "dump FAIL: %s: %s\n", a_filename, a_title);
	}
	close (fd);
}

//------------------------------------------------------------------------------
// @brief load tree from file
void Lettvin::Table::load (const char* a_filename)
{
	int32_t fd = open (a_filename, O_RDONLY, 0);
	if (fd >= 0)
	{
	}
	close (fd);
}

//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

//------------------------------------------------------------------------------
/// @brief ctor
Lettvin::GreasedGrep::
GreasedGrep (int32_t a_argc, char** a_argv) // ctor
//------------------------------------------------------------------------------
{
	while (--a_argc)
	{
		//debugf (1, "CTOR: '%s' '%s'\n", *a_argv, *(a_argv + 1));
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
		Lettvin::syntax ("pattern(s) and directory required.");
	}

	// Find filename regexes
	debugf (1, "Initial target: %s\n", s_target.data ());
	size_t brace = s_target.find_first_of ('{');
	if (brace != string_view::npos)
	{
		string patterns{s_target.substr (brace)};
		s_target = s_target.substr (0, brace);
		tokenize (s_filesx, patterns, "{,}");
		for (size_t I=s_filesx.size (), i=1; i<I; ++i)
		{
			auto& restriction{s_filesx[i]};
			s_regex.emplace_back (regex (restriction.data ()));
			debugf (1, "Target restriction: '%s'\n", restriction.data ());
		}
	}
	debugf (1, "Final target: %s\n", s_target.data ());

	// Validate ingested args
	if (s_accept.size () < 2 && s_reject.size () < 2)
	{
		syntax ("specify at least one accept or reject str");
	}

	// Check for valid directory
	if (!fs::is_directory (s_target) &&
		!fs::is_regular_file (s_target))
	{
		syntax ("last arg must be dir or file");
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
		dump ("temp.dump", "Trial dump");
		show_tables (cout);
		show_tokens (cout);
	}
	debugf (1, "FIRSTS: %s\n", s_firsts.c_str ());

	bool https {s_target.substr (0, 8) == "https://"};
	bool http  {s_target.substr (0, 7) == "http://"};
	bool ftp   {s_target.substr (0, 6) == "ftp://"};
	if (https || http || ftp)
	{
		netsearch (s_target);
	}
	else if (fs::is_directory (s_target))
	{
		// Find files and search contents
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
		syntax ("(%s) options must precede other args", a_str.data ());
	}
	if (bad)
	{
		syntax ("command-line options must be two or more chars");
	}

	// Doing debug first is special
	//s_debug += (a_str == "--debug");
	//s_debug += (two && (letter == 'd'));

	if (a_str.size () == 2 && isdigit(a_str[1]))
	{
		s_oversize = (size_t)(a_str[1] - '0');
		s_oversize = s_oversize ? s_oversize : 1;
		debugf (1, "CORE MULTIPLIER (%u)\n", s_oversize);
		return true;
	}

	debugf (1, "OPTION: preparse: '%s'\n", a_str.data ());

	if      (a_str == "--case"     || (opt && letter == 'c')) s_caseless = false;
	else if (a_str == "--debug"    || (opt && letter == 'd')) s_debug   += 1;
	else if (a_str == "--nibbles"  || (opt && letter == 'n')) s_nibbles  = true;
	else if (a_str == "--suppress" || (opt && letter == 's')) s_suppress = true;
	else if (a_str == "--test"     || (opt && letter == 't')) s_test     = true;
	else if (a_str == "--variant"  || (opt && letter == 'v')) s_variant  = true;
	else if (a_str[0] == a_str[1] && a_str[1] == '-')
	{
		debugf (1, "OPTIONS:\n");
		for (size_t I=a_str.size (), i=2; i < I; ++i)
		{
			char opt{a_str[i]};
			debugf (1, "OPTION: '%c'\n", opt);
			switch (opt)
			{
				case 'c': option ("-c"); break;
				case 'd': option ("-d"); break;
				case 'n': option ("-n"); break;
				case 's': option ("-s"); break;
				case 't': option ("-t"); break;
				case 'v': option ("-v"); break;
				default:
					syntax ("OPTION: '%c' is illegal", a_str[i]);
					break;
			}
		}
	}
	else if (opt)
	{
		syntax ("unknown arg '%s'", a_str.data ());
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
	bool minus2{size >= 2 && a_str[0] == '-' && a_str[1] == '-'};

	debugf (1, "INGEST: %d %d %s\n", minus1, minus2, a_str.data ());

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
			syntax ("pattern strings must be longer than 1 byte");
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
compile (int32_t a_sign)
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
compile (int32_t a_sign, string_view a_sv)
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

	// TODO setindex is used as an indirect forward reference from an Atom.
	// s_set[setindex] are the direct forward references of candidates which
	// terminate on a particular Atom.
	// When using variants, the probability of token identity drops with
	// each addition to the set.
	size_t setindex = s_set.size ();
	s_set.resize (setindex + 1);
	set<int32_t>& setterminal{s_set[setindex]};  ///< insert 1st & variant terminals

	vector<string> variant_names;

	// Initially, the string as given is searched
	// TODO generate variants like levenshtein, fatfinger
	// TODO handle collision for variants
	// TODO generate all NFKD variants for insertion
	// e.g. "than" and "then" are legitimate mutual variants.
	vector<string> strs;
	bool caseless{s_caseless};
	strs.emplace_back (a_str);

	if (s_variant)
	{
		size_t bracket_init{a_sv.find_first_of ('[')};
		if (bracket_init != string_view::npos)
		{
			size_t bracket_fini{a_sv.find_first_of (']', bracket_init + 1)};
			if (bracket_fini == string_view::npos)
			{
				syntax ("bracketd variants");
			}
			debugf (1, "BR %zu %zu\n", bracket_init, bracket_fini);
			b_str = a_sv.substr (bracket_init + 1, bracket_fini - bracket_init - 1);
			a_str = a_sv.substr (0, bracket_init);
			debugf (1, "Bracketed [%s][%s]\n", a_str.c_str (), b_str.c_str ());
			// TODO use utility.h tokenize here
			size_t comma = b_str.find_first_of (',');
			while (comma != string::npos)
			{
				string token = b_str.substr (0, comma);
				b_str = b_str.substr (comma + 1);
				comma = b_str.find_first_of (',');
				is_variant (token.c_str ())
					? register_variant (variant_names, token)
					: descramble_variants (variant_names, token);
			}
			if (b_str.size ())
			{
				is_variant (b_str.c_str ())
					? register_variant (variant_names, b_str)
					: descramble_variants (variant_names, b_str);
			}
		}

		// Have all specified variant functions add to the alternatives.
		for (auto& variant:variant_names)
		{
			const auto& generator_iter = s_variant_generator.find (variant);
			if (generator_iter != s_variant_generator.end ())
			{
				generator_iter->second (strs, a_str);
			}
		}
	}

	// Reduce the alternatives to the set of uniques
	set<string> unique;
	for (auto& item:strs) unique.insert (item);
	strs.clear ();
	for (auto& item:unique) strs.emplace_back (item);

	// Insert variants into transition tree
	for (auto& str: strs)
	{
		insert (str, id, setindex);
	}
	s_caseless = caseless;
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
	int32_t fd;

	if (s_regex.size ())
	{
		// TODO return if filename pattern does not match a filesx
		bool matched{false};
		for (auto& matcher:s_regex)
		{
			matched |= regex_search (a_filename, matcher);
		}
		if (!matched)
		{
			return;
		}
	}

	try
	{
		fd = open (a_filename, O_RDONLY, 0);
	}
	catch (...)
	{
		debugf (1, "MAPPED_SEARCH: file open failed\n", a_filename);
		return;
	}

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
			int32_t rc = munmap (contents, filesize);
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
/// @brief run search on incoming packets
void
Lettvin::GreasedGrep::
netsearch (string_view a_URL)
{
	a_URL = a_URL;
}

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
		uint32_t cpus{thread::hardware_concurrency () * s_oversize};
		vector<thread> threads;
		Lettvin::ThreadedQueue<string> tq (cpus + 1);  // 1 ahead of workers
		// Note worker threads are 1 less than cpus
		for (uint32_t thid = 1; thid < cpus; thid++)
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
						fs::path path (item.path ());
						try
						{
							path = fs::canonical (path);
						}
						catch (...)
						{
							debugf (1,
									"WALK CANONICAL failed (%s)\n",
									path.c_str ());
							continue;
						}
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
main (int32_t a_argc, char** a_argv)
//------------------------------------------------------------------------------
{
	try
	{
		std::ios::sync_with_stdio (true);
		//fs::path app{fs::canonical (fs::path (a_argv[0]))};
		fs::path app{fs::path (a_argv[0])};
		Lettvin::s_path = const_cast<const char*>(app.c_str ());
		Lettvin::GreasedGrep gg (a_argc, a_argv);
		gg ();
		Lettvin::debugf (1, "CTOR: '%s' '%s'\n", *a_argv, *(a_argv + 1));
	}
	catch (const std::exception &e)
	//catch (...)
	{
		Lettvin::synopsis ("Unexpected signal caught (%s).", e.what ());
	}
	return 0;
} // main
#endif
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

