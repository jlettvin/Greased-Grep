#pragma once

#include <vector>
#include <string_view>

#include "gg_version.h"

namespace Lettvin
{
	using namespace std;

	const char* s_path=".";

	//--------------------------------------------------------------------------
	/// @brief synopsis (document usage in case of failure)
	void synopsis (const char* a_message, ...)
	//--------------------------------------------------------------------------
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

	//--------------------------------------------------------------------------
	/// @brief synopsis (document usage in case of failure)
	void syntax (const char* a_message, ...)
	//--------------------------------------------------------------------------
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

	//__________________________________________________________________________
	/// @brief convert a (delimited?) view with separators into a vector of views
	///
	/// target is where separated views go
	/// source is the view to be tokenized
	/// sep is either a char or a const char* of separators
	/// oc  is a const char* of delimiters.
	/// example 1: tokenize (myvector, "[hello world]", ' ', "[]");
	/// example 2: tokenize (myvector, "this,that and another", " ,");
	string::size_type
	tokenize (
			vector<string>& target,
			string a_source,
			string separator=" ")
	{
		string::size_type N = a_source.size ();
		string::size_type truncate = string::npos;
#if 1
		string source;
		char sep;
		switch (separator.size ())
		{
			case 1:
				source = a_source;
				sep = separator[0];
				break;
			case 3:
				if (separator[0] == a_source[0] && separator[2] == a_source[N-1])
				{
					source = a_source.substr (1, N-2);
					sep = separator[1];
				}
				else
				{
					synopsis ("TOKENIZE: bad delimiters '%s'", separator.data ());
				}
				break;
			default:
				target.push_back ("FOO");
				return 0;
				synopsis ("TOKENIZE: bad delimiters '%s'", separator.data ());
		}
		string::size_type b = 0, e = source.find_first_of (sep);
		while (e != string::npos)
		{
			string token(source.substr (b, e-b));
			target.push_back (token);
			b = e+1;
			e = source.find_first_of (sep, b);
		}
		target.push_back (source.substr (b));
#else
		debugf (1, "TOKENIZE[I]: '%s'\n", a_source.data ());
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
#endif
		return truncate;
	}
}
