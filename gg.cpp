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

//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

//..............................................................................
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
#include "gg_utility.h"            // tokenize
#include "gg_tqueue.h"             // filename distribution to threads
#include "gg_state.h"              // Mechanism for finite state machine
#include "gg.h"                    // declarations

//..............................................................................
#include "gg_variant.h"            // variant implementations


//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

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
	else
	{
		// Find files and search contents
		walk (s_target);
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
	bool nbls   {s_shape.nibbles ()};
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

	bool l_nibbles{false};

	if      (a_str == "--case"     || (opt && letter == 'c')) s_caseless = false;
	else if (a_str == "--debug"    || (opt && letter == 'd')) s_debug   += 1;
	else if (a_str == "--nibbles"  || (opt && letter == 'n')) l_nibbles  = true;
	else if (a_str == "--quicktree"|| (opt && letter == 'q')) s_quicktree= true;
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
	if (l_nibbles && !nbls)
	{
		/// This is where command-line option -n causes plane size to change.
		s_shape (true);
	}
	return true;
}

//------------------------------------------------------------------------------
/// @brief ingest inserts state-transition table data
//
// For now, gg fails when GG_COMPILE is defined
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
#ifndef GG_COMPILE
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

#ifdef GG_COMPILE
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

	TODO(setindex is used as an indirect forward reference from an Transition.)
	// s_set[setindex] are the direct forward references of candidates which
	// terminate on a particular Transition.
	// When using variants, the probability of token identity drops with
	// each addition to the set.
	size_t setindex = insert (a_str, id);
	set<int32_t>& setterminal{s_set[setindex]};  ///< 1st & variant terminals

	vs_t variant_names;

	vs_t strs;
	bool caseless{s_caseless};

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
			TODO(use gg_utility.h tokenize here)
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

	if (s_quicktree)
	{
		printf ("gg:quicktree (%s)\n", a_filename);
		return;
	}

	if (s_regex.size ())
	{
		TODO(return if filename pattern does not match a filesx)
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
		if (!s_suppress)
		{
			printf ("gg:mapped_search OPEN EXCEPTION: %s\n", a_filename);
		}
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
			if (!s_suppress)
			{
				printf ("gg:mapped_search MAP FAILED: %s\n", a_filename);
			}
		}
		else
		{
			track (contents, filesize, a_filename);
			int32_t rc = munmap (contents, filesize);
			if (rc != 0) synopsis ("munmap failed");
		}

		close (fd);
	}
	else if (!s_suppress)
	{
		printf ("gg:mapped_search OPEN FAILED: %s\n", a_filename);
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
walk (const string& a_path)
{
	// Don't attempt to assess validity of filenames... just fail.
	// Treat directories like files and search filenames in directories.
	// This enables gg to work on sshfs mounted filesystems.
	auto d{a_path};
	auto s{d.size ()};
	if (s && d[s - 1] == '/') d.resize (s-1);
	errno = 0;
    if (auto dir = opendir (d.c_str ()))
	{
		while (!errno)
		{
			if (auto f = readdir (dir))
			{
				if (auto p = f->d_name)
				{
					if (auto q = p)
					{
						if (!(*q++ == '.' && (!*q || (*q++ == '.' && !*q))))
						{
							auto e = d + "/" + p;
							walk (e);
							mapped_search (e.c_str ());
							errno = 0;
						}
					}
					else break;
				}
				else break;
			}
			else break;
		}
	}
}

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
