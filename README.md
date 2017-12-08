# Greased-Grep (version 0.0.314)
## High performance O(N) case insensitive UTF8<br />threaded recursive fuzzy search for files having<br />all of one set and none of another set of strings
### $ gg copyright -Lettvin .       # find files with copyright and without Lettvin
### $ gg +smile +joy -frown -sad .  # files filled with nothing but happiness
### $ gg 愚公移山 .                 # find the foolish old man who moved mountains

"gg" name was chosen for this frequently-used command because it is easy to type.
"gg" is the left-hand index-finger letter, and typing it twice is easy.
"gfg" (greased-fgrep) would be more accurate since fgrep is not regex.
But "gfg" is harder to type than "gg" so this is the decided app name.

Fuzzy searching is under development but is similar to case insensitivity.

<hr />

### Copyright

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
```

<hr />

### Usage

```
Greased Grep version 0.0.1

USAGE: gg [-c] [-n] [-s] [-t] [+|-]{str} [[+|-]{str}...] {path} 

Greased Grep UTF8 search for files having (case insensitive):
    all instances of +{str} or {str} and
	no  instances of -{str} instances in
	files found along {path}

        {str} are simple strings (no regex).
        {str} may be single-quoted to avoid shell interpretation.

ARGUMENTS:  all search strings must be 2 characters or more

    [+]{str}      # add accept string (+ optional)

    -{str}        # add reject string

    {path}        # file or top directory for recursive search

ARGUMENT OPTIONS: (TODO)

    When the --variant option is used
    A {str} followed by a bracket-list triggers variant insertion

    Examples:
       $ gg -v copyright[acronym,c,f,soundex,nyssis] .
    Available:
       a or acronym           to insert variants like M.I.T.
       c or contraction       to insert variants like MIT
       f or fatfinger         to insert variants like NUR
       l or levenshtein1      to insert variants like MTI
       m or metaphone         may dismiss as post-processing
       n or nyssis            may dismiss as post-processing
       s or soundex           may dismiss as post-processing
       t or thesaurus         to insert synonyms
       u or unicode           to insert NFKD variants

OPTIONS:

    -c, --caseless  # case sensitive search

    -d, --debug     # turn on debugging output (multiple increases level)

    -n, --nibbles   # use nibbles (lower memory use half-speed search)

    -s, --suppress  # suppress permission denied errors

    -t, --test      # test algorithms (unit and timing)  TODO

    -v, --variant   # enable variant syntax with [] brackets  TODO

OUTPUT:

    canonical paths of files fulfilling the set conditions.

EXAMPLES:

    $ gg include /usr/local/src
        # find all files having the string 'inlude' in /usr/local/src

    $ gg '#include <experimental/filesystem>' /usr/local/src
        # find all files having the quoted string in /usr/local/src

    $ gg copyright -Lettvin .
        # Find all files with missing or other than Lettvin copyright.

    $ gg 愚公移山 .
        # Find the foolish old man who moved the mountains
```

<hr />

The last example is a good illustration.
Here is is more specialized.


```
$ ./gg copyright -IBM brief -nevermore .
```

This command reads:


```
    Greased Grep for files containing
    "copyright" and "brief" and
    not "IBM" and not "nevermore"
	in the tree from "." down.
```

On stackoverflow I requested an equivalent grep command-line and received:
https://stackoverflow.com/questions/47562028/how-to-caseless-grep-listing-names-of-files-containing-all-of-several-strings-an

```
$ grep -FlirZ 'copyright' . \
    | xargs -0 grep -FliZ 'brief' \
    | xargs -0 grep -FLi -e 'IBM' -e 'nevermore'
```

This is an excellent approach and still
Greased-Grep will likely outperform it.
It will be compared when performance testing is integrated.

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
* Use "//CCCCCC..." repeated to column 79 or 80 above a class
* align "public", "protected", and "private with open brace
* short classes can be compressed vertically to fit in a single editor window
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

### Tail comparison (TODO)
Once subsequent chars must be unique, a memicmp outperforms the FSM.

### FSM (Finite State Machine)
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
<ul>

 <li>
implement variant generation/insertion
 <table>
  <tr><td>a or acronym</td>     <td>to insert variants like M.I.T.</td></tr>
  <tr><td>c or contraction</td> <td>to insert variants like MIT</td></tr>
  <tr><td>f or fatfinger</td>   <td>to insert variants like NUR</td></tr>
  <tr><td>l or levenshtein1</td><td>to insert variants like MTI</td></tr>
  <tr><td>m or metaphone</td></tr>
  <tr><td>n or nyssis</td></tr>
  <tr><td>s or soundex</td></tr>
  <tr><td>t or thesaurus</td>   <td>to insert synonyms</td></tr>
  <tr><td>u or unicode</td>     <td>to insert NFKD variants</td></tr>
 </table>
 </li>

 <li>
Modify reversion.py to update README and gg_version.h to the same designation.
 </li>

 <li>
measure performance against fgrep/ack/ag
  <ul>
   <li>
publishing performance will make gg more attractive
   </li>
  </ul>
 </li>

 <li>
ingest args with ctor but compile strs at beginning of ftor
  <ul>
   <li>
compilation in ftor currently fails
   </li>
  </ul>
 </li>

 <li>
use memcmp for unique final string
  <ul>
   <li>
When the tail end of a search is unique memcmp is faster.
This optimization increases complexity considerably.
   </li>
  </ul>
 </li>

 <li>
implement self-test (-t)
  <ul>
   <li>
client-usable as opposed to unit-test and performance test
   </li>
  </ul>
 </li>

 <li>
translate UTF8->UnicodeCodepoint->NFKD->UnicodeCodepoint->UTF8
  <ul>
   <li>
strings with identical appearance should be comparable
this could be done by decomposing and recomposing during compilation
for instance; convert to Unicode Codepoints, and decompose, then
recompose to canonical NFKD, then reconvert to UTF8, then
strings so recomposed can be compared properly
   </li>
  </ul>
 </li>
</ul>

<hr />

## GOALS
Search can be made fuzzy

<hr />
