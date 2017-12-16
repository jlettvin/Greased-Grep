#pragma once

#include <vector>
#include <string>
#include <cstdarg>
#include <string_view>

#include "gg_globals.h"
#include "gg_version.h"

namespace Lettvin
{
	using namespace std;

	//--------------------------------------------------------------------------
	void
	noop () {}
	//--------------------------------------------------------------------------

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
	void
	syntax (const char* a_message, ...)
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

	//--------------------------------------------------------------------------
	int
	debugf (size_t a_debug, const char *fmt, ...)
	//--------------------------------------------------------------------------
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
			vs_t& target,
			string a_source,
			string separator=" ")
	{
		string::size_type N = a_source.size ();
		string::size_type truncate = string::npos;
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
		return truncate;
	}
}
