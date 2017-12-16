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

/*
 * A standard plane consumes 32 bits (4 bytes) * 256 = 1KiB/plane.
 * A nibbles  plane consumes 32 bits (4 bytes) *  16 =  64B/plane.
 * For a given unique character 2 nibbles planes are needed for each 1 standard.
 * This means a standard place consumes memory at 8 times the rate of nibbles.
 * Also, use of nibbles decreases search speed by a factor of 2.
 * This is because 2 nibbles plane dereferences are needed.
 */

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
#include "utility.h"               // Finite State Machine
#include "state_table.h"           // Finite State Machine
#include "gg_version.h"            // version

#define SETINDIRECT true

namespace fs = std::experimental::filesystem;

namespace Lettvin
{
	using namespace std;  // No naming collisions in this small namespace

	//--------------------------------------------------------------------------
	void nibbles ();                            ///< convert to nibbles not bytes
	//--------------------------------------------------------------------------

	typedef int32_t       i24_t;
	static_assert (sizeof(int32_t) == 4);

	bool    s_caseless {true};            ///< case sensitivity initially false
	bool    s_nibbles  {false};           ///< nibble planes replace bute planes
	bool    s_suppress {false};           ///< suppress error messages

	bool    s_noreject {true};            ///< are there reject strings?
	bool    s_test     {false};           ///< run unit and timing tests
	bool    s_variant  {false};           ///< enable variant syntax

	uint8_t s_root    {1};                ///< syntax tree root plane number

	size_t s_mask    {0xffULL};
	size_t s_prefill {1};
	size_t s_size    {256};

	uint32_t s_oversize{1};

	// This union gives a guaranteed order for little and big endian bytes.
	// It is used in Table dump/load functions.
	// In data written to a file, either endian refers to the same byte when
	// s_order.u08[i] for i between 0 and 7.
	// Files may be written in either endian order.
	// To dump/load is requirement for thesaurus which will be stored as Table.
	static const union {
		unsigned long long                  u64;
		struct { unsigned long  array[2]; } u32;
		struct { unsigned short array[4]; } u16;
		struct { unsigned char  array[8]; } u08;
	} s_order { .u64=0x0706050403020100 };

	double s_overhead;                   ///< interval for noop

	string s_firsts;                     ///< string of {arg} first letters
	string s_target;

	vector<regex>           s_regex;         ///< filename match regexes
	vsv_t                   s_accept {{""}}; ///< list of accept {str} args
	vsv_t                   s_reject {{""}}; ///< list of reject {str} args
	vs_t                    s_filesx {{""}}; ///< list of filename regex patterns
	vector< set<int32_t> >  s_set    {{ 0}}; ///< per-candidate sets

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
		void walk (const fs::path& a_path);

		//----------------------------------------------------------------------
		void show_tokens (ostream& a_os);

	}; // class GreasedGrep

} // namespace Lettvin
