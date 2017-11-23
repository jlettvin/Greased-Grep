# Greased-Grep
## High performance case insensitive search for files<BR /> having all of one set and none of another set of strings
```
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

Usage: gg [+|-]{str} [[+|-]{str}...] {path} 
Greased Grep search for files having (case insensitive):
    all instances of +{str} or {str} and
	no  instances of -{str} instances in
	files found along {path}

{str} are simple strings (no regex).
{str} may be single-quoted to avoid shell interpretation.

Examples:

    gg include /usr/local/src

    gg '#include <experimental/filesystem>' /usr/local/src

    gg copyright -Lettvin .
        # Find all files with missing or other than Lettvin copyright.
```


The last example is a good illustration.
Here is is more specialized.

```
./gg copyright -IBM brief -nevermore .
```

This command reads:

```
    Greased Grep for
    "copyright" and "brief"
    but ignore any containing either
    "IBM" or "nevermore"
	for all files in the tree from . down.
```

## Algorithm
This code uses two significant optimizations on string searches:
* skipping of irrelevant initial characters
* FSM (Finite-State-Machine) table-driven with very low overhead
* memory-mapped files

### Skipping
C++ strings have a 'find_first_of' method optimizing skip to a valid initial char.

### FSM
Memory is cheap (modern idiom) so
very sparse tables are generated for
rapid state transitions.
Redundancy opportunities are taken by
re-use of tables with common preceding characters.

### Memory Mapped files
No buffering or data copying is required so
overhead for indexing characters is fairly low.
Tests have shown ambiguous time optimization
but code simplification merits this choice.
