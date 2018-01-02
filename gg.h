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

#define EXPERIMENTAL_FILESYSTEM false

/*
 * A standard plane consumes 32 bits (4 bytes) * 256 = 1KiB/plane.
 * A nibbles  plane consumes 32 bits (4 bytes) *  16 =  64B/plane.
 * For a given unique character 2 nibbles planes are needed for each 1 standard.
 * This means a standard place consumes memory at 8 times the rate of nibbles.
 * Also, use of nibbles decreases search speed by a factor of 2.
 * This is because 2 nibbles plane dereferences are needed.
 */

//..............................................................................
#if EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem> // recursive directory walk
namespace fs = std::experimental::filesystem;
#else
#include <dirent.h>
#endif

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
#include <functional>              // support the interval function
#include <iostream>                // sync_with_stdio (mix printf with cout)
#include <iomanip>                 // setw and other cout formatting
#include <cstdarg>                 // vararg
#include <regex>                   // filename matching

//..............................................................................
#include <string>                  // container
#include <vector>                  // container
#include <chrono>                  // steady_clock
#include <map>                     // container
#include <set>                     // container

//..............................................................................
#include "catch.hpp"               // Testing framework

//..............................................................................
#include "gg_globals.h"
#include "gg_utility.h"            // Finite State Machine
#include "gg_state.h"              // Finite State Machine
#include "gg_version.h"            // version

namespace Lettvin
{
	using namespace std;  // No naming collisions in this small namespace

	//--------------------------------------------------------------------------
	void nibbles ();                            ///< convert to nibbles not bytes
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	/// @brief measure time to perform a function
	template<typename T>
	double interval (T a_fun, const char* a_message=nullptr)
	//--------------------------------------------------------------------------
	{
		chrono::steady_clock::time_point t0 = chrono::steady_clock::now ();
		a_fun ();
		chrono::steady_clock::time_point t1 = chrono::steady_clock::now ();
		chrono::duration<double> time_span =
			chrono::duration_cast<chrono::duration<double>>(t1 - t0);

		double seconds  = time_span.count ();

		if (a_message)
		{
			cout
				<< " # gg interval: " << seconds
				<< " seconds for " << a_message
				<< endl;
		}
		return seconds;
	}

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	//__________________________________________________________________________
	/// @brief GreasedGrep implements overall state-transition operations
	class
	GreasedGrep : public Table
	//__________________________________________________________________________
	{
		friend class Table;
	//------
	public:
	//------

		//----------------------------------------------------------------------
		/// @brief ctor
		GreasedGrep (int32_t a_argc, char** a_argv); // ctor

		//----------------------------------------------------------------------
		/// @brief ftor
		void operator ()();

	//------
	private:
	//------

		//----------------------------------------------------------------------
		/// @brief option handles command-line long options
		///
		/// @returns true for a successfully handled arg
		bool option (string_view a_str);

		//----------------------------------------------------------------------
		/// @brief ingest handles command-line options and arguments.
		void ingest (string_view a_str);

		//----------------------------------------------------------------------
		/// @brief compile inserts state-transition table data
		void compile (int32_t a_sign=0);

		//----------------------------------------------------------------------
		/// @brief compile a single string argument
		///
		/// Distribute characters into state tables for searching.
		void compile (int32_t a_sign, string_view a_str);

		//----------------------------------------------------------------------
		/// @brief run search on incoming packets
		void netsearch (string_view a_URL);

		//----------------------------------------------------------------------
		/// @brief map file into memory and call search
		///
		/// https://techoverflow.net/2013/08/21/a-simple-mmap-readonly-example/
		void mapped_search (const char* a_filename);

		//----------------------------------------------------------------------
		/// @brief walk organizes search for strings in memory-mapped file
#if EXPERIMENTAL_FILESYSTEM
		void walk (const fs::path& a_path);
#else
		void walk (const string& a_path);

		bool is_directory    (const string& a_path);
		bool is_regular_file (const string& a_path);
#endif // EXPERIMENTAL_FILESYSTEM

		//----------------------------------------------------------------------
		void show_tokens (ostream& a_os);

	}; // class GreasedGrep

} // namespace Lettvin
