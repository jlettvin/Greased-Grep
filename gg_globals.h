#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <set>

#include <regex>

namespace Lettvin
{
	using namespace std;

	//__________________________________________________________________________
	typedef int32_t             i24_t;
	typedef unsigned            integral_t;

	typedef vector<string>      vs_t;
	typedef vector<string_view> vsv_t;

	static_assert (sizeof(int32_t)    == 4);
	static_assert (sizeof(integral_t) == 4);

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
}
