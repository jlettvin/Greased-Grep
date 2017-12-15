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

#include <cstdarg>                 // vararg
#include <vector>
#include <string>
#include <tuple>

int32_t debugf (size_t a_debug, const char *fmt, ...);

#include "utility.h"

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
