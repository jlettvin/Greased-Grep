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

#include "gg_globals.h"

namespace Lettvin
{
	//__________________________________________________________________________
	const char* s_path=".";
	size_t      s_debug    {0};

	bool        s_caseless {true};      ///< case sensitivity initially false
	bool        s_suppress {false};     ///< suppress error messages

	bool        s_noreject {true};      ///< are there reject strings?
	bool        s_test     {false};     ///< run unit and timing tests
	bool        s_variant  {false};     ///< enable variant syntax

	bool        s_quicktree{false};     ///< just show the filenames

	uint8_t     s_root     {1};         ///< syntax tree root plane number

	Shape       s_shape;

	uint32_t s_oversize{1};

	double   s_overhead;                ///< interval for noop

	string   s_firsts;                  ///< string of {arg} first letters
	string   s_target;

	vector<regex>           s_regex;         ///< filename match regexes
	vsv_t                   s_accept {{""}}; ///< list of accept {str} args
	vsv_t                   s_reject {{""}}; ///< list of reject {str} args
	vs_t                    s_filesx {{""}}; ///< list of filename regex patterns
	vector< set<int32_t> >  s_set    {{ 0}}; ///< per-candidate sets

	// This union gives a guaranteed order for little and big endian bytes.
	// It is used in Table dump/load functions.
	// In data written to a file, either endian refers to the same byte when
	// s_order.u08[i] for i between 0 and 7.
	// Files may be written in either endian order.
	// To dump/load is requirement for thesaurus which will be stored as Table.
	//const union {
		//unsigned long long                  u64;
		//struct { unsigned long  array[2]; } u32;
		//struct { unsigned short array[4]; } u16;
		//struct { unsigned char  array[8]; } u08;
	//} s_order { .u64=0x0706050403020100 };

} // namespace Lettvin
