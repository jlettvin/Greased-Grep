#!/usr/bin/env python

# _____________________________________________________________________________
#             The MIT License (https://opensource.org/licenses/MIT)
# 
#         Copyright (c) 2017, Jonathan D. Lettvin, All Rights Reserved.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# _____________________________________________________________________________

import re
import sys

def synopsis ():
    print sys.argv[0]
    print """
revision.py updates a file containing a specific version line pattern.
    "static const struct { unsigned major, minor, build; } = {M,m,b};"
    where M=Major, m=minor, b=build are integers.
After extracting the build number, it increments it, then writes the file again.

USAGE: revision.py {filename}
"""
    exit (1);

#    The line to be replaced in gg.h is:
# static const struct { unsigned major, minor, build; } s_version{0,0,2};

################################################################################
#    Both the input and output matched line share a common prefix:
common = "static const struct { unsigned major, minor, build; } ";
#    Search pattern for identifying the correct line in gg.h
pattern = common + "s_version{(\d+),(\d+),(\d+)};"
#    Output format for generating the replacement line for gg.h
format  = common + "s_version{%d,%d,%d};"
#    Prepare for searching as many lines as necessary.
prog = re.compile(pattern)
################################################################################


################################################################################
def main (filename):
    # output will contain the list of lines to write (with build incremented)
    output = []

    # Pull the lines in
    with open (filename) as source:
        input  = source.read ().splitlines ()

    # Find the line needing replacement
    for line in input:
        m = re.search(prog, line)
        if m:
            major, minor, build = [
                    int (m.group (1)),
                    int (m.group (2)),
                    int (m.group (3))]
            output += [format % (major, minor, build+1)]
        else:
            output += [line]

    # Push the lines out
    with open (filename, "w") as target:
        for line in output:
            print >> target, line
################################################################################


################################################################################
if __name__ == "__main__":
    if len (sys.argv) != 2:
        synopsis ()
    else:
        main (sys.argv[1])
################################################################################
