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

#include <cstdint>
#include <vector>
#include <string>
#include <set>

#include <regex>

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x))

#define SIZED_TYPEDEF(o,n,s) typedef o n; static_assert (sizeof (o) == s)

namespace Lettvin
{
	using namespace std;

	//__________________________________________________________________________
	SIZED_TYPEDEF(int32_t , i24_t     ,4);
	SIZED_TYPEDEF(unsigned, integral_t,4);

	typedef vector<string>      vs_t;
	typedef vector<string_view> vsv_t;

	//__________________________________________________________________________
	extern size_t      s_debug   ;

	extern bool        s_caseless;      ///< case sensitivity initially false
	extern bool        s_nibbles ;      ///< nibble planes replace bute planes
	extern bool        s_suppress;      ///< suppress error messages

	extern bool        s_noreject;      ///< are there reject strings?
	extern bool        s_test    ;      ///< run unit and timing tests
	extern bool        s_variant ;      ///< enable variant syntax

	extern uint8_t     s_root    ;      ///< syntax tree root plane number

	extern size_t      s_mask    ;
	extern size_t      s_prefill ;
	extern size_t      s_size    ;

	extern uint32_t    s_oversize;

	extern double      s_overhead;      ///< interval for noop

	extern string      s_firsts  ;      ///< string of {arg} first letters
	extern string      s_target  ;
	extern const char* s_path    ;

	//__________________________________________________________________________
	extern vector<regex>           s_regex ; ///< filename match regexes
	extern vsv_t                   s_accept; ///< list of accept {str} args
	extern vsv_t                   s_reject; ///< list of reject {str} args
	extern vs_t                    s_filesx; ///< list of filename regex patterns
	extern vector< set<int32_t> >  s_set   ; ///< per-candidate sets

	//__________________________________________________________________________
	// This union gives a guaranteed order for little and big endian bytes.
	// It is used in Table dump/load functions.
	// In data written to a file, either endian refers to the same byte when
	// s_order.u08[i] for i between 0 and 7.
	// Files may be written in either endian order.
	// To dump/load is requirement for thesaurus which will be stored as Table.
	//extern order_t s_order;
	//extern const union {
		//unsigned long long                  u64;
		//struct { unsigned long  array[2]; } u32;
		//struct { unsigned short array[4]; } u16;
		//struct { unsigned char  array[8]; } u08;
	//} s_order;
    void nibbles ();
}
