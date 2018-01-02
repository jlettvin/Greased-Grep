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
#include "gg.h"                    // declarations

//..............................................................................


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
#if EXPERIMENTAL_FILESYSTEM
		//fs::path app{fs::canonical (fs::path (a_argv[0]))};
		fs::path app{fs::path (a_argv[0])};
#else
		std::string app{a_argv[0]};
#endif // EXPERIMENTAL_FILESYSTEM
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

