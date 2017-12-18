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

GSRC=\
	gg.cpp \
	gg_globals.cpp \
	gg_utility.cpp \
	gg_tqueue.cpp \
	gg_state.cpp

CSRC=$(GSRC)

GOBJ=\
	gg_globals.o \
	gg_utility.o \
	gg_tqueue.o \
	gg_state.o

COBJ=$(GOBJ)

GHDR=\
	gg_version.h \
	gg_globals.h \
	gg_utility.h \
	gg_tqueue.h \
	gg_state.h \
	gg_variant.h \
	gg.h

CHDR=catch.hpp $(GOBJ)

ARCHIVE=\
	.gitignore \
	LICENSE \
	README.md \
	Makefile \
	$(CSRC) \
	$(CHDR) \
	gg_test.cpp \
	test

EMPTY=
REJECT=-$(EMPTY)m$(EMPTY)n$(EMPTY)o
################################################################################

# TODO remove bug forcing GG_COMPILE in ingest rather than ftor
GG_COMPILE=
#GG_COMPILE=-DGG_COMPILE
CDEBUG=-g -ggdb -O0
CFINAL=-O3
CXX=g++
CXXFLAGS=\
	-std=c++17 \
	-Wextra -Wall \
	-Wno-unused-variable \
	$(GG_COMPILE) \
	$(CDEBUG)

# Removed -Werror to ignore warnings
LOPTS=-pthread -lfmt -lstdc++fs
CEXES=gg gg_test make_README
#CEXES=gg gg_tqueue make_README
################################################################################

################################################################################
all: before $(CEXES) test README.md after

################################################################################
#	./gg -d ab . > artifact/ab.art
#	./gg -d xy . > artifact/xy.art
#	./gg -c -d -n AA .
#	./gg_tqueue
.PHONY:
test: FORCE
	./gg -s abc def ghi jkl .
	./gg -s -d +abc +def +ghi +jkl $(REJECT) .
	./gg_test


################################################################################
README.md:	make_README
	./make_README > README.md

################################################################################
make_README: make_README.cpp $(CHDR) Makefile
	$(CXX) $(CXXFLAGS) -o $@ $< $(LOPTS)

################################################################################
# gg_test.cpp main is used in place of gg.cpp when -DGG_TEST is defined
gg_test: gg_test.cpp $(COBJ) $(CHDR) Makefile
	$(CXX) \
		-o $@ \
		-DGG_TEST \
		$(CXXFLAGS) \
		$(COBJ) \
		gg.cpp \
		gg_test.cpp \
		$(LOPTS)

################################################################################
gg: $(COBJ) $(CHDR) Makefile
	$(CXX) \
		-o $@ \
		$(CXXFLAGS) \
		$(COBJ) \
		gg.cpp \
		gg_main.cpp \
		$(LOPTS)

################################################################################
#gg_tqueue: gg_tqueue.cpp gg_tqueue.h
#	@echo "Test gg_tqueue"
#	g++ -DMAIN $(CXXFLAGS) -o $@ $< $(LOPTS)

################################################################################
#	g++ -O3 -Wextra -Wall -Werror -std=c++17 -o gg gg.cpp -lfmt -lstdc++fs
#	$(CXX) $(CXXFLAGS) -o $@ $< $(LOPTS)
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
	rm -f *.o
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
