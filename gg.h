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

//..............................................................................
#include <string>                  // container
#include <vector>                  // container
#include <chrono>                  // steady_clock
#include <set>                     // container

//..............................................................................
#include "catch.hpp"               // Testing framework

//..............................................................................
#include "gg_version.h"            // version

namespace fs = std::experimental::filesystem;

namespace Lettvin
{
	using namespace std;  // No naming collisions in this small namespace

	//--------------------------------------------------------------------------
	void synopsis (const char* a_message, ...); ///< report errors and exit
	void nibbles ();                            ///< convert to nibbles not bytes
	int debugf (size_t a_debug, const char *fmt, ...);
	//--------------------------------------------------------------------------

	typedef unsigned char u08_t;
	typedef int           i24_t;

	bool   s_caseless{true};             ///< turn on case insensitivity
	bool   s_nibbles {false};            ///< nibble planes replace bute planes
	bool   s_suppress{false};            ///< suppress error messages

	bool   s_noreject{true};             ///< are there reject strings?
	bool   s_test    {false};            ///< run unit and timing tests

	u08_t  s_root    {1};                ///< syntax tree root plane number

	size_t s_debug   {0};
	size_t s_mask    {0xffULL};
	size_t s_prefill {1};
	size_t s_size    {256};

	double s_overhead;                   ///< interval for noop

	string s_firsts;                     ///< string of {arg} first letters
	string_view s_target;

	vector<string_view> s_accept {{""}}; ///< list of accept {str} args
	vector<string_view> s_reject {{""}}; ///< list of reject {str} args

	//--------------------------------------------------------------------------
	void noop () {}
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

		double seconds  = time_span.count();

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
	/// @brief Single state-transition element.
	///
	/// constructor/getters/setters for atomic element unit.
	//__________________________________________________________________________
	class
	Atom
	//__________________________________________________________________________
	{
	//------
	public:
	//------
		Atom () {}                                               ///< ctor
		u08_t tgt (         ) const { return  m_the.state.tgt; } ///< tgt getter
		i24_t str (         ) const { return  m_the.state.str; } ///< str getter
		void  tgt (u08_t a_tgt)     { m_the.state.tgt = a_tgt; } ///< tgt setter
		void  str (i24_t a_str)     { m_the.state.str = a_str; } ///< str setter
	//------
	private:
	//------
		union {
			unsigned short integral;                   ///< unused name
			struct { i24_t str:24; u08_t tgt; } state; ///< strtern/state ids
		} m_the
		{.integral=0};
	}; // class Atom

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition plane of 256 Atoms
	///
	/// constructor/indexer
	//__________________________________________________________________________
	class
	State
	//__________________________________________________________________________
	{
	//------
	public:
	//------
		State () : m_handle (s_size)      {                         } ///< ctor
		Atom& operator[] (u08_t a_off)    { return m_handle[a_off]; } ///< index
	//------
	private:
	//------
		vector<Atom> m_handle;
	}; // class State

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief vector of state-transition planes sufficient to enable search
	///
	/// constructor/indexer
	//__________________________________________________________________________
	class
	Table
	//__________________________________________________________________________
	{
	//------
	public:
	//------

		//----------------------------------------------------------------------
		/// @brief Table ctor (reserve many, instance 2)
		Table ();

		//----------------------------------------------------------------------
		/// @brief indexer
		State& operator[] (u08_t a_offset);

		//----------------------------------------------------------------------
		/// @brief add State planes to vector
		void operator++ ();

		//----------------------------------------------------------------------
		/// @brief return current size of vector
		size_t size ();

		//----------------------------------------------------------------------
		/// @brief debug utility for displaying the entire table
		ostream& show_tables (ostream& a_os);

		//----------------------------------------------------------------------
		/// @brief insert strings into tables
		void insert (string_view a_str, i24_t id);

		//----------------------------------------------------------------------
		/// @brief find and report found strings
		///
		/// when reject list is empty, terminate on completion of accept list
		/// when reject list is non-empty, terminate on first reject
		void search (void* a_pointer, auto a_bytecount, const char* a_label="");

	//--------
	protected:
	//--------

		//dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
		vector<State> m_table;                       ///< State tables
		State&        m_state1{operator[] (s_root)}; ///< root state plane

	//------
	private:
	//------

		//----------------------------------------------------------------------
		/// @brief insert either case-sensitive or both case letters into tree
		///
		/// Distribute characters into state tables for searching.
		void insert (
				char* a_chars,
				auto& a_from,
				auto& a_next,
				bool a_stop=false,
				bool a_nibbles=false);

	}; // class Table

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
		GreasedGrep (int a_argc, char** a_argv); // ctor

		//----------------------------------------------------------------------
		/// @brief ftor
		void operator() ();

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
		void compile (int a_sign=0);

		//----------------------------------------------------------------------
		/// @brief compile a single string argument
		///
		/// Distribute characters into state tables for searching.
		void compile (int a_sign, string_view a_str);

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
