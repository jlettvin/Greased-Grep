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
		debugf (1, "CONTRACTION[%s] %s\n",
				a_phrase.c_str (), a_target[0].c_str ());
#if false
    def bool_algorithm_contraction(self, canon, rough):
        # Handle identity.
        if canon == rough:
            return self.bool_report(True, 'contraction', rough, canon)
        self.generate_head_tail_indices(canon, rough)
        rlen = len(rough)
        less = rlen - 2
        if rlen == self.head:
            return self.bool_report(True, 'contraction', rough, canon)
        if self.head >= 2:
            return self.bool_report(True, 'contraction', rough, canon)
        if self.both > less:
            return self.bool_report(True, 'contraction', rough, canon)
        return False
#endif
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t     ellipses (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "ELLIPSES[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
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
            'P': 'PpO0-[;Lo)_{:l',
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
            'A': 'Aa?:,Oo;\'',
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
            'L': 'LlRr6@8*/&SsNn',
            'M': 'MmBbHhTtWw ',
            'N': 'NnTtRrLlSsVvWw',
            'O': 'OoAa,.EeQq;\'',
            'P': 'Pp.3)1"YyUuEe',
            'Q': 'Qq\':OoEeJj ',
            'R': 'RrCc4$6@LlNnTt',
            'S': 'SsNnLl&/-ZzVv',
            'T': 'TtHhCcRrNnWwMm',
            'U': 'UuEePpYyIiKkJj',
            'V': 'VvWwNnSsZz',
            'W': 'Ww MmTtNnVv',
            'X': 'Xx KkIiDdBb',
            'Y': 'YyPp1"9%FfIiUu',
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
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t levenshtein1 (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "LEVENSHTEIN1[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
#if false
    def bool_algorithm_Levenshtein1(self, canon, rough):
        # Handle identity.
        if canon == rough:
            return self.bool_report(True, 'Levenshtein1', rough, canon)
        self.generate_head_tail_indices(canon, rough)
        # Handle length difference out-of-range.
        if abs(self.Clen-self.Rlen) > 1:
            return False
        # Handle deletion and insertion.
        if self.Clen != self.Rlen:
            return self.Nmin == self.both
        # Handle a single typo.
        if self.diff == 1:
            return self.bool_report(True, 'Levenshtein1', rough, canon)
        # Handle 1 swapped pair.
        diagonal1 = canon[   self.head] == rough[-1-self.tail]
        diagonal2 = canon[-1-self.tail] == rough[   self.head]
        if diagonal1 and diagonal2:
            return self.bool_report(True, 'Levenshtein1', rough, canon)
        return False
#endif
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t  misspelling (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "MISSPELLING[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t    thesaurus (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "THESAURUS[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	//__________________________________________________________________________
	size_t      unicode (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "UNICODE[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
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
		{"misspelling" , misspelling}	,{"m", misspelling},
		{"thesaurus"   , thesaurus}		,{"t", thesaurus},
		{"unicode"     , unicode}		,{"u", unicode}
	};

}
