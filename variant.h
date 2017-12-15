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

#pragma once

#include <vector>
#include <string>
#include <map>

#include <functional>

#include <fmt/printf.h>            // modern printf

#include "gg.h"

// TODO variants won't fully work until target indirection is implemented

namespace Lettvin
{
	using namespace std;

	//__________________________________________________________________________
	size_t acronym (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "ACRONYM[%s] %s\n",
				a_phrase.c_str (), a_target[0].c_str ());
		// If a_phrase is "Massachusetts Institute of Technology"
		string plain;    // plain  is "MIT"
		string pspace;   // pspace is "M I T"
		string dotted;   // dotted is "M.I.T."
		string dspace;   // dspace is "M. I. T."

		static const vector<string> exclude{"of", "the"};
		size_t count{0};
		size_t space = a_phrase.find_first_of (' ');
		while (space != string::npos)
		{
			string token{a_phrase.substr (0, space)};
			while (' ' == a_phrase[++space]);
			a_phrase = a_phrase.substr (space);
		    space = a_phrase.find_first_of (' ');

			auto iter{find (exclude.begin (), exclude.end (), token)};
			bool keep{iter != exclude.end ()};
			if (keep) continue;

			char c{token[0]};
			plain  += c;
			pspace += c; pspace += ' ';
			dotted += c; dotted += '.';
			dspace += c; dspace += '.'; dspace += ' ';
			++count;
		}
		if (count && a_phrase.size ())
		{
			char c{a_phrase[0]};
			plain  += c;
			pspace += c; pspace += ' ';
			dotted += c; dotted += '.';
			dspace += c; dspace += '.'; dspace += ' ';
			++count;
		}
		if (count)
		{
			a_target.emplace_back (plain);
			a_target.emplace_back (pspace);
			a_target.emplace_back (dotted);
			a_target.emplace_back (dspace);
		}

		debugf (1, "ACRONYM PLAIN  '%s'\n",  plain.c_str ());
		debugf (1, "ACRONYM PSPACE '%s'\n", pspace.c_str ());
		debugf (1, "ACRONYM DOTTED '%s'\n", dotted.c_str ());
		debugf (1, "ACRONYM DSPACE '%s'\n", dspace.c_str ());

		return count;
	}

	//__________________________________________________________________________
	size_t  contraction (vector<string>& a_target, string a_phrase)
	{
		static string vowel{"AaEeIiOoUu"};
		static size_t limit{2}; // Am, Amer, but not Americ for America
		debugf (1, "CONTRACTION[%s] %s\n",
				a_phrase.c_str (), a_target[0].c_str ());
		size_t syllables{0};
		for (size_t N=a_phrase.size (), n=1;n < N; ++n)
		{
			auto found{vowel.find (a_phrase[n])};
			if (found == string::npos) continue;
			if (++syllables > limit) break;
			a_target.emplace_back (a_phrase.substr (0, n));  // Amer
			debugf (1, "CONTRACTION PLAIN  '%s'\n", a_target.back ().c_str ());
			a_target.emplace_back (a_phrase.substr (0, n));  // Amer. (period)
			auto& token{a_target.back ()};
			token += '.';
			debugf (1, "CONTRACTION DOTTED '%s'\n", a_target.back ().c_str ());
		}
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t     ellipses (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "ELLIPSES[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t limit{8};  // smallest string for ellipses cut
		for (size_t N=a_phrase.size () - 1, n=1;n < N; ++n)
		{
			if (n >= limit)
			{
				a_target.emplace_back (a_phrase.substr (0, n));
				debugf (1, "ELLIPSES '%s'\n", a_target.back ().c_str ());
			}
		}
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t    fatfinger (vector<string>& a_target, string a_phrase)
	{
#if false
# Map qwerty keyboard to possible fat_fingerings.
    QWERTY = {
            # These are lists of all keys within a one key radius of center.
            # Only alphabetics are considered for centers and
            # only uppercase are considered for matching (by using .upper()).
            'A': 'AaQWSXZqwsxz',
            'B': 'BbVGHNvghn',
            'C': 'CcXDFVxdfv',
            'D': 'DdSERFCXserfcx',
            'E': 'EeWSDR34#$wsdr',
            'F': 'FfDRTGVCdrtgvc',
            'G': 'GgFTYHBVftyhbv',
            'H': 'HhGYUJNBgyujnb',
            'I': 'IiUJKO89(*ujko',
            'J': 'JjHUIKMNhuikmn',
            'K': 'KkJIOL<Mjiol,m',
            'L': 'LlKOP:><kop;.,',
            'M': 'MmNJKL< jkl,',
            'N': 'NnBHJM bhjm',
            'O': 'OoI90PLK()iplk',
            'P': 'PpO0-[\x3bLo\x7d_\x7b:l',
            'Q': 'Qq  12WA!@wa',
            'R': 'Rr45TFDE$%tfde',
            'S': 'SsAWEDXZawedxz',
            'T': 'TtR56YGFr%^ygf',
            'U': 'UuY78IJH&*ijh',
            'V': 'VvCFGB cfgb',
            'W': 'WwQ23ESAq@#esa',
            'X': 'XxZSDC zsdc',
            'Y': 'YyT67UHGt^&uhg',
            'Z': 'ZzASXasx',
    }

    # Map Dvorak keyboard to possible fat_fingerings.
    DVORAK = {
            'A': 'Aa?:,Oo\x3b\'',
            'B': 'BbXxDdHhMm ',
            'C': 'CcGg24$$4TtHh',
            'D': 'DdIiFfGgHhBbXx',
            'E': 'EeOo.PpUuJjQq',
            'F': 'FfYy9%0_GgDdIi',
            'G': 'GgFf0_2CcHhDd',
            'H': 'HhDdGgCcTtMmBb',
            'I': 'IiUuYyFfDdXxKk',
            'J': 'JjQqEeUuKk ',
            'K': 'KkJjUuIiXx ',
            'L': 'LlRr6\x408\x2a/&SsNn',
            'M': 'MmBbHhTtWw ',
            'N': 'NnTtRrLlSsVvWw',
            'O': 'OoAa,.EeQq\x3b\'',
            'P': 'Pp.3\x291\x22YyUuEe',
            'Q': 'Qq\':OoEeJj ',
            'R': 'RrCc4\x246@LlNnTt',
            'S': 'SsNnLl&/-ZzVv',
            'T': 'TtHhCcRrNnWwMm',
            'U': 'UuEePpYyIiKkJj',
            'V': 'VvWwNnSsZz',
            'W': 'Ww MmTtNnVv',
            'X': 'Xx KkIiDdBb',
            'Y': 'YyPp1\x229%FfIiUu',
            'Z': 'ZzVvSs-',
    }

    keyboard = {
            # This dictionary enables consideration of alternate keyboards.
            'QWERTY': QWERTY,
            'DVORAK': DVORAK
            }

    stopwords = [u'THE', u'OF', u'AND', u'FOR', u'INC', u'--']
#endif
		debugf (1, "FATFINGER[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		synopsis ("FATFINGER unimplemented");
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t levenshtein1 (vector<string>& a_target, string a_phrase)
	{
		//debugf (1, "LEVENSHTEIN1[%s] %s\n", 
				//a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		size_t I0{a_target.size ()};
		size_t N{a_phrase.size () - 1};
		// Missing terminal chars
		if (N > 3)
		{
			a_target.emplace_back (a_phrase.substr (1));
			a_target.emplace_back (a_phrase.substr (0, N-1));
			// Missing character doubled character, and flipped characters
			for (size_t n=1; n <= N; ++n)
			{
				a_target.emplace_back ( // missing
						a_phrase.substr (0,n) +
						a_phrase.substr (n+1));
				debugf (1, "LEV1 missing '%s'\n", a_target.back ().c_str ());
				a_target.emplace_back ( // doubled
						a_phrase.substr (0,n+1) +
						a_phrase.substr (n));
				debugf (1, "LEV1 double '%s'\n", a_target.back ().c_str ());
				a_target.emplace_back ( // flipped
						a_phrase.substr (0,n-1) +
						a_phrase.substr (n, 1) +
						a_phrase.substr (n-1, 1) +
						a_phrase.substr (n+1));
				debugf (1, "LEV1 flipped '%s'\n", a_target.back ().c_str ());
			}
		}
		for (size_t I1=a_target.size (), i=I0; i < I1; ++i)
		{
			debugf (1, "LEVENSHTEIN1[%s] %s\n",
					a_phrase.c_str (),
					a_target[i].c_str ());
		}
		return count;
	}

	//__________________________________________________________________________
	size_t    sensitive (vector<string>& a_target, string a_phrase)
	{
		// This flips global case sensitivity for the individual phrase.
		// It gets turned back on again, if the global flag was on.
		debugf (1, "CASE SENSITIVE[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		s_caseless ^= true; // GreasedGrep::compile saves and restores this flag.
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t    thesaurus (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "THESAURUS[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		synopsis ("THESAURUS SYNONYMS unimplemented");
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t      unicode (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "UNICODE[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		synopsis ("UNICODE NFKD unimplemented");
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	typedef size_t (*variantp_t)(vector<string>&, string);
	typedef map<string, variantp_t> mapvariant_t;

	mapvariant_t s_variant_generator{
		{"acronym"     , acronym}		,{"a", acronym},
		{"contraction" , contraction}	,{"c", contraction},
		{"ellipses"    , ellipses}		,{"e", ellipses},
		{"fatfinger"   , fatfinger}		,{"f", fatfinger},
		{"levenshtein1", levenshtein1}	,{"l", levenshtein1},
		{"sensitive"   , sensitive}	,    {"s", sensitive},
		{"thesaurus"   , thesaurus}		,{"t", thesaurus},
		{"unicode"     , unicode}		,{"u", unicode}
	};

	//__________________________________________________________________________
	bool is_variant (const char* id)
	{
		mapvariant_t::const_iterator citer;
		citer = s_variant_generator.find (id);
		return (citer != s_variant_generator.end ());
	}

	//__________________________________________________________________________
	void synopsis_if_not_variant (const char* id)
	{
		if (!is_variant (id))
		{
			synopsis ("VARIANT: bad '%s'", id);
		}
	}

	//__________________________________________________________________________
	bool register_variant (vector<string>& registry, string id)
	{
		registry.push_back (id);
		debugf (1, "variant: '%s'\n", id);
		return true;
	}

	//__________________________________________________________________________
	bool descramble_variants (vector<string>& variant_names, string scrambled)
	{
		for (size_t I=scrambled.size (), i=0; i<I; ++i)
		{
			string single (scrambled.substr (i,1));
			debugf (1, "VARIANT descramble '%s'", single);
			synopsis_if_not_variant (single.c_str ());
			register_variant (variant_names, single);
		}
		return true;
	}
}
