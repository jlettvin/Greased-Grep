#pragma once

#include <vector>
#include <string>

namespace Lettvin
{
	using namespace std;

	vector<string> acronym (string a_phrase)
	{
		vector<string> result;
		size_t space = a_phrase.find_first_of (' ');
		while (space)
		{
			result.emplace_back (a_phrase.substr (0, space));
			while (' ' == a_phrase[++space]);
			a_phrase = a_phrase.substr (space);
			result.push_back (string (1, toupper (a_phrase[0])));
			result.back () += '.';
		}
		if (a_phrase.size ())
		{
			result.push_back (string (1, toupper (a_phrase[0])));
			result.back () += '.';
		}
		return result;
	}
}
