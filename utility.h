#pragma once

#include <vector>
#include <string_view>

namespace Lettvin
{
	using namespace std;

	//__________________________________________________________________________
	/// @brief convert a (delimited?) view with separators into a vector of views
	///
	/// target is where separated views go
	/// source is the view to be tokenized
	/// sep is either a char or a const char* of separators
	/// oc  is a const char* of delimiters.
	/// example 1: tokenize (myvector, "[hello world]", ' ', "[]");
	/// example 2: tokenize (myvector, "this,that and another", " ,");
	template<typename T>
	string::size_type
	tokenize (
			vector<string>& target,
			string_view a_source,
			const T sep=' ',
			const char *oc="\0\0")
	{
		debugf (1, "TOKENIZE[I]: '%s'\n", a_source.data ());
		string::size_type truncate{string::npos};
		string source{a_source};
		if (oc[0])
		{
			auto e = source.find_first_of (oc[1]);
			truncate = e;
			if (source[0] != oc[0])
			{
				synopsis ("TOKENIZE: missing '%s'[0]", oc);
			}
			if (e == string_view::npos)
			{
				synopsis ("TOKENIZE: missing '%s'[1]", oc);
			}
			source = a_source.substr (1, e-1);
			//source.remove_prefix (1);
			//e = source.find_first_of (oc[1]);
			//source.remove_suffix (e-1);
			debugf (1, "TOKENIZE[S]: '%s' %zu %zu\n",
				source.c_str (),
				source.size (),
				e);
		}
		string::size_type b{0};
		string::size_type e{0};
		while ((b = source.find_first_not_of (sep, b)) != string_view::npos)
		{
			e = source.find_first_of (sep, b);
			target.emplace_back (string_view (source.substr (b, e-b-1)));
			b = e;
			debugf (1, "TOKENIZE[B]: '%s'\n", target.back ().data ());
		}
		if (e != string_view::npos && e != source.size ())
		{
			target.emplace_back (
					string_view (source.substr (e, source.size () - 1)));
			debugf (1, "TOKENIZE[A]: '%s'\n", target.back ().data ());
		}
		return truncate;
	}
}
