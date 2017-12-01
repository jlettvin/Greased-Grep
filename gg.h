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
#include <iomanip>                 // setw and other cout formatting

//..............................................................................
#include <string>                  // container
#include <vector>                  // container
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
	/// @brief synopsis (document usage in case of failure)
	void synopsis (const char* a_message);
	void nibbles ();
	//--------------------------------------------------------------------------

	typedef unsigned char u08_t;
	typedef int           i24_t;

	bool   s_nibbles{false};
	size_t s_prefill {1};
	size_t s_mask    {0xffULL};
	size_t s_size    {256};

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

	//------
	private:
	//------

		vector<State> m_table; ///< State tables

	}; // class Table

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	//__________________________________________________________________________
	/// @brief GreasedGrep implements overall state-transition operations
	class
	GreasedGrep : public Table
	//__________________________________________________________________________
	{
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
		/// @brief ingest inserts state-transition table data
		void ingest (string_view a_str);

		//----------------------------------------------------------------------
		/// @brief ingest inserts state-transition table data
		void compile (int a_sign=0);

		//----------------------------------------------------------------------
		/// @brief insert either case-sensitive or both case letters into tree
		///
		/// Distribute characters into state tables for searching.
		/// TODO nibbles handling is under development and may not work yet.
		void insert (
				char* a_chars,
				auto& a_from,
				auto& a_next,
				bool a_stop=false,
				bool a_nibbles=false);

		//----------------------------------------------------------------------
		/// @brief compile a single string argument
		///
		/// Distribute characters into state tables for searching.
		void compile (int a_sign, string_view a_str);

		//----------------------------------------------------------------------
		/// @brief find and report found strings
		///
		/// when reject list is empty, terminate on completion of accept list
		/// when reject list is non-empty, terminate on first reject
		void search (void* a_pointer, auto a_bytecount, const char* a_label="");

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

		//dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
		vector<string_view> m_accept {{""}}; ///< list of accept {str} args
		vector<string_view> m_reject {{""}}; ///< list of reject {str} args
		u08_t m_root{1};                     ///< syntax tree root plane number
		size_t m_debug{0};                   ///< turns on verbosity
		bool m_noreject{true};               ///< are there reject strings?
		bool m_suppress{false};              ///< suppress error messages
		bool m_caseless{true};               ///< turn on case insensitivity
		bool m_nibbles {false};              ///< use small-plane nibble code
		bool m_test    {false};              ///< run unit and timing tests

		string_view m_directory;
		string m_firsts;                     ///< string of {arg} first letters
		State& m_state1{operator[] (m_root)};///< root state plane
	}; // class GreasedGrep

} // namespace Lettvin
