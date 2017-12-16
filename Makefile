#!/usr/bin/env make
# Makefile for building greased grep
#_____________________________________________________________________________
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

################################################################################
#DEFINES=
#SHARED_OPTS=-std=c++17 -I. -Wall -Wextra -Werror -fPIC -ggdb
#GOPTS=-g -O0 -coverage -pg -fprofile-arcs -ftest-coverage
#ALLOPTS=$(DEFINES) $(SHARED_OPTS) $(GOPTS)
#CPPFLAGS=$(ALLOPTS)wrap printf
#LFLAGS=-lgcov --coverage -lfmt -lstdc++fs
#LFLAGS=-lfmt -lstdc++fs
################################################################################

################################################################################
DATETIME=`date +%Y%m%d%H%M%S`
CSRC=gg.cpp gg_tqueue.cpp gg_state.cpp gg_test.cpp
CHDR=\
	 catch.hpp \
	 gg_globals.h gg.h gg_version.h \
	 gg_tqueue.h gg_state.h gg_utility.h gg_variant.h
ARCHIVE=.gitignore LICENSE Makefile README.md $(CSRC) $(CHDR) test
EMPTY=
REJECT=-$(EMPTY)m$(EMPTY)n$(EMPTY)o
################################################################################
CC=g++
COPTS_BOTH=-Wextra -Wall -Werror -std=c++17
COPTS_DEBUG=-g -ggdb -O0 $(COPTS_BOTH)
COPTS_FINAL=-O3  $(COPTS_BOTH)
COPTS=$(COPTS_DEBUG)
LOPTS=-pthread -lfmt -lstdc++fs
CEXES=gg gg_test make_README
#CEXES=gg gg_tqueue make_README
################################################################################

################################################################################
all: before $(CEXES) test README.md after

################################################################################
.PHONY:
test: FORCE
	./gg -d ab . > artifact/ab.art
	./gg -d xy . > artifact/xy.art
	./gg -s abc def ghi jkl .
	./gg -s -d +abc +def +ghi +jkl $(REJECT) .
	./gg -c -d -n AA .
	./gg_test

#	./gg_tqueue

################################################################################
README.md:	make_README
	@./make_README > README.md

################################################################################
make_README: make_README.cpp $(CHDR) Makefile
	$(CC) $(COPTS) -o $@ $< $(LOPTS)

################################################################################
gg_test:	gg_test.cpp gg_state.cpp $(CHDR) Makefile
	$(CC) $(COPTS) -o $@ gg_globals.cpp gg_test.cpp gg_state.cpp $(LOPTS)

################################################################################
gg:	gg.cpp gg_state.cpp $(CHDR) Makefile
	$(CC) $(COPTS) -o $@ gg_globals.cpp gg.cpp gg_state.cpp $(LOPTS)

################################################################################
#gg_tqueue: gg_tqueue.cpp gg_tqueue.h
#	@echo "Test gg_tqueue"
#	g++ -DMAIN $(COPTS) -o $@ $< $(LOPTS)

################################################################################
#	g++ -O3 -Wextra -Wall -Werror -std=c++17 -o gg gg.cpp -lfmt -lstdc++fs
#	$(CC) $(COPTS) -o $@ $< $(LOPTS)
#	@g++ -O3 -Wextra -Wall -Werror -std=c++17 -o gg gg.cpp -lfmt -lstdc++fs
#	@strip gg
#	@./gg -s abc def ghi jkl .
#	@./gg -s +abc +def +ghi +jkl $(REJECT) .
################################################################################

################################################################################
.PHONY:
before:    FORCE
	@printf "\n\t\t\t\t\tMAKE\n"
	@echo $$(yes '-' | head -n 40)
	@seq 3 | xargs -I -- echo $$(yes 'v' | head -n 40)
	@echo $$(yes '-' | head -n 40)
	@./reversion.py gg_version.h

################################################################################
.PHONY:
after:    FORCE
	@echo "$ gede --args gg -d -n aa test   # to debug"
	@echo $$(yes '-' | head -n 40)
	@seq 3 | xargs -I -- echo $$(yes '^' | head -n 40)
	@echo $$(yes '-' | head -n 40)

################################################################################
.PHONY:
clean:    FORCE
	rm -fr $(CEXES)
	rm -f gg_tqueue
	rm -f *.out
	rm -f temp.dump
	rm -f *.gcov *.gcda *.gcno

################################################################################
.PHONY:
tgz:    FORCE
	@mkdir -p tgz
	@tar cvzf tgz/gg.$(DATETIME).tgz $(ARCHIVE)

################################################################################
FORCE:
