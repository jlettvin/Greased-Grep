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

//..............................................................................
// g++ --std=c++17 -o gg_test gg_test.cpp
#define CATCH_CONFIG_MAIN
#include "catch.hpp"               // Testing framework

#include <sstream>
#include <cstdarg>                 // vararg
#include <vector>
#include <string>
#include <tuple>

int32_t debugf (size_t a_debug, const char *fmt, ...);

#include "gg_utility.h"
#include "gg_state.h"

using namespace std;
using namespace Lettvin;

//______________________________________________________________________________
SCENARIO ("Simple test of catch.hpp")
{
	GIVEN ("CATCH_CONFIG_MAIN is defined and catch.hpp is included")
	{
		THEN ("We should pass a trivial test")
		{
			REQUIRE (true == true);
		}
	}
}

//______________________________________________________________________________
SCENARIO ("Test utility functions")
{
	GIVEN ("Some test data to exercise the tokenizer")
	{
		const vector<string> expect{{"a","bb","ccc"}};
		vector<tuple<string,string,string::size_type>>
		candidates{{
			{"[a,bb,ccc]", "[,]", string::npos},
			{ "a bb ccc" , " "  , string::npos},
			{"{a:bb:ccc}", "{:}", string::npos}
		}};

		THEN ("Test tokenize")
		{
			for (auto& candidate:candidates)
			{
				vector<string> target;
				auto source = get<0>(candidate);
				auto sep    = get<1>(candidate);
				auto ret = tokenize (target, source, sep);
				REQUIRE (target == expect      );
				//REQUIRE (ret    == string::npos);
			}
		}
	}
}

//______________________________________________________________________________
SCENARIO ("Test Transition, State, and Table")
{
	GIVEN ("Constructed Transition")
	{
		Transition transition;
		THEN ("Test Transition target API")
		{
			REQUIRE (transition.integral () == 0); // test ctor empty

			INFO ("Just one");
			transition.tgt (1);
			REQUIRE (transition.tgt() == 1);
			REQUIRE (transition.integral () != 0);

			INFO ("Try all");
			for (size_t tgt=1;tgt<256;++tgt)
			{
				transition.tgt (tgt);
				REQUIRE (transition.tgt() == tgt);
				REQUIRE (transition.integral () != 0);
			}
			transition.tgt (0);
			REQUIRE (transition.tgt() == 0);
			REQUIRE (transition.integral () == 0);
		}

		THEN ("Test Terminal string API")
		{
			int top = (1 << 23) - 1;
			for (i24_t str=1;str<top;++str)
			{
				transition.str (str);
				REQUIRE (transition.str () == str);
				transition.str (-str);
				REQUIRE (transition.str () == -str);
				REQUIRE (transition.integral () != 0);
			}
			transition.str (0);
			REQUIRE (transition.str () == 0);
			REQUIRE (transition.integral () == 0);
		}
	}

#if 0
	GIVEN ("State construction and operation of API")
	{
		THEN ("Test State")
		{
			State state;

			for (size_t source=0; source<256; ++source)
			{
				REQUIRE (state[source].integral () == 0); // test ctor empty
				REQUIRE (state[source].integral () == 0);
				state[source].tgt ('B');
				REQUIRE (state[127].integral () != 0);
				state[127].tgt (0);
				REQUIRE (state[127].integral () == 0);
			}
		}
		THEN ("Test Table")
		{
			Table table;
			REQUIRE (table.size () == 2);
			++table;
			table++;
			REQUIRE (table.size () == 4);
		}
	}
#endif
}

//______________________________________________________________________________
SCENARIO ("Test Table")
{
	GIVEN ("Some test data to exercise the Table")
	{
		const string empty (R"table( # 
 #    00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15
 #  ________________________________________________________________________________     PLANE: 0
 # |________________________________________________________________________________|
 # 
 #    00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15
 #  ________________________________________________________________________________     PLANE: 1
 # |________________________________________________________________________________|
)table");
		typedef tuple<vs_t,string> tvs_t;
		const vector<tvs_t> expect{
			{{"a","b"}, ""}
		};
		for (auto& candidate:expect)
		{
			stringstream ss;
			auto tokens = get<0>(candidate);
			auto output = get<1>(candidate);

			Table table;

			// Test the empty table for empty contents
			table.show_tables (ss);
			const string& result (ss.str ());
			REQUIRE (result == empty);
			ss.clear ();

			size_t index{0};
			for (auto& token:tokens)
			{
				++index;
				INFO ("TOKEN: " << index << " is '" << token << "'");
				//table.insert (token, index, index);
			}

			// After table.insert... test again
		}
	}
}

