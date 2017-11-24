# Greased-Grep
## High performance case insensitive search for files<br /> having all of one set and none of another set of strings
### $ gg copyright -Lettvin .  # find copyright files without Lettvin
<hr />

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

<hr />

The last example is a good illustration.
Here is is more specialized.


```
./gg copyright -IBM brief -nevermore .
```

This command reads:


```
    Greased Grep for files containing
    "copyright" and "brief" and
    not "IBM" and not "nevermore"
	in the tree from "." down.
```

<hr />
This code has been tested on:

* ubuntu linux 16.04 with kernel version 4.10.0-38
* g++ version 7.1.0

Contributors are welcome to port this to different systems
and offer pushes for me to pull.
Prefer readability of code over cleverness or elegance.
If a block of code deserves attribution
make sure to do right by its original author.
If a block of code is difficult
make sure to provide references to document what the code does.
Please attempt to keep to the code style:

* nominal doxgen is used (or more)
* class members begin with "m_" and have suggestive names.
* method/function arguments begin with "a_".
* vertically surround the return-type signature in dashed comments (//-----)
* put return-type on a line before the method/function signature.
* beginning brace is after the 2nd dashed comment, aligned with function name
* similar rules are used for class declaration/definition but
* Follow a close-brace with the name of the object (method/function/class)
** Use "//CCCCCC..." repeated to column 79 or 80 above a class
** align "public", "protected", and "private with open brace
** short classes can be compressed vertically to fit in a single editor window
* for a small compilation unit, the declaration and definition may be combined
<hr />

## C++17
This code depends on C++17 for experimental libraries.

### string_view
This optimizes string operations over std::string.

### fmt::printf
This introduces thread-safe printf.

### experimental/filesystem
This is a push of boost::filesystem into std::.

<hr />

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

### TODO
* TODO fix that the final arg is a directory, otherwise synopsis.
* TODO handle canonicalization problem.
* TODO find bug for when m_table is not reserved

<hr />
