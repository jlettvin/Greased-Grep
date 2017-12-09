#pragma once

#include <vector>
#include <string>

#include <fmt/printf.h>            // modern printf

#include "gg.h"

namespace Lettvin
{
	using namespace std;

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

	size_t  contraction (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "CONTRACTION[%s] %s\n",
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t     ellipses (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "ELLIPSES[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t    fatfinger (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "FATFINGER[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t levenshtein1 (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "LEVENSHTEIN1[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t    metaphone (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "METAPHONE[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t       nyssis (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "NYSSIS[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t      soundex (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "SOUNDEX[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t    thesaurus (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "THESAURUS[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

	size_t      unicode (vector<string>& a_target, string a_phrase)
	{
		debugf (1, "NFKD[%s] %s\n", 
				a_phrase.c_str (), a_target[0].c_str ());
		size_t count{0};
		return count;
	}

}
