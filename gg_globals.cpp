#include "gg_globals.h"

namespace Lettvin
{
	//__________________________________________________________________________
	const char* s_path=".";
	size_t      s_debug    {0};

	bool        s_caseless {true};      ///< case sensitivity initially false
	bool        s_nibbles  {false};     ///< nibble planes replace bute planes
	bool        s_suppress {false};     ///< suppress error messages

	bool        s_noreject {true};      ///< are there reject strings?
	bool        s_test     {false};     ///< run unit and timing tests
	bool        s_variant  {false};     ///< enable variant syntax

	uint8_t     s_root     {1};          ///< syntax tree root plane number

	size_t      s_mask     {0xffULL};
	size_t      s_prefill  {1};
	size_t      s_size     {256};

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

}

