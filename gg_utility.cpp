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

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

#include <stdarg.h>
#include <fmt/format.h>

#include "gg_utility.h"

void
Lettvin::
noop ()
{
}

void
Lettvin::
synopsis (const char* a_message, ...)
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

void
Lettvin::
syntax (const char* a_message, ...)
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
} // syntax

int
Lettvin::
debugf (size_t a_debug, const char *fmt, ...)
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

int
Lettvin::
assertf (bool a_flag, size_t a_debug, const char *fmt, ...)
{
	static const char* indent{"  "};
	int32_t ret = 0;
	if (s_debug >= a_debug && !a_flag)
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
} // assertf

int
Lettvin::
logf (const char *fmt, ...)
{
	int32_t ret = 0;
	static const mode_t mode{S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH};
	{
		va_list args;
		va_start (args, fmt);
		/*
		int fd = open ("gg.log", O_CREAT | O_APPEND, mode);
		if (fd >= 0)
		{
			ret = vdprintf (fd, fmt, args);
			close (fd);
		}
		*/
		va_end (args);
	}
	return ret;
} // logf

std::string::size_type
Lettvin::
tokenize (
	vs_t& target,
	std::string a_source,
	std::string separator)
{
	std::string::size_type N = a_source.size ();
	std::string::size_type truncate = string::npos;
	std::string source;
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
		std::string token(source.substr (b, e-b));
		target.push_back (token);
		b = e+1;
		e = source.find_first_of (sep, b);
	}
	target.push_back (source.substr (b));
	return truncate;
} // tokenize
