#!/usr/bin/env python

import re

#    The line to be replaced in gg.h is:
# static const struct { unsigned major, minor, build; } s_version{0,0,1};

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
def main ():
    # output will contain the list of lines to write (with build incremented)
    output = []

    # Pull the lines in
    with open("gg.h") as source:
        input  = source.read().splitlines()

    # Find the line needing replacement
    for line in input:
        m = re.search(prog, line)
        if m:
            major, minor, build = [
                    int(m.group(1)),
                    int(m.group(2)),
                    int(m.group(3))]
            output += [format % (major, minor, build+1)]
        else:
            output += [line]

    # Push the lines out
    with open("gg.h", "w") as target:
        for line in output:
            print>>target, line
################################################################################


################################################################################
if __name__ == "__main__":
    main()
################################################################################
