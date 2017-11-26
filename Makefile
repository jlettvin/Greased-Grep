#!/usr/bin/env make
# Makefile for building greased grep
# 

################################################################################
#DEFINES=
#SHARED_OPTS=-std=c++17 -I. -Wall -Wextra -Werror -fPIC -ggdb
#GOPTS=-g -O0 -coverage -pg -fprofile-arcs -ftest-coverage
#ALLOPTS=$(DEFINES) $(SHARED_OPTS) $(GOPTS)
#CPPFLAGS=$(ALLOPTS)
#LFLAGS=-lgcov --coverage -lfmt -lstdc++fs
#LFLAGS=-lfmt -lstdc++fs
################################################################################

################################################################################
DATETIME=`date +%Y%m%d%H%M%S`
ARCHIVE=.gitignore LICENSE Makefile README.md gg.cpp test
EMPTY=
REJECT=-$(EMPTY)m$(EMPTY)n$(EMPTY)o
################################################################################
CC=g++
COPTS=-O3 -Wextra -Wall -Werror -std=c++17
LOPTS=-lfmt -lstdc++fs
COBJS=gg
################################################################################

################################################################################
all: before gg test after

################################################################################
.PHONY:
test: FORCE
	@./gg -d ab . > artifact/ab.art
	@./gg -d xy . > artifact/xy.art
	@./gg -s abc def ghi jkl .
	@./gg -s -d +abc +def +ghi +jkl $(REJECT) .

################################################################################
gg:	gg.cpp Makefile
	$(CC) $(COPTS) -o gg gg.cpp $(LOPTS)

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

################################################################################
.PHONY:
after:    FORCE
	@echo $$(yes '-' | head -n 40)
	@seq 3 | xargs -I -- echo $$(yes '^' | head -n 40)
	@echo $$(yes '-' | head -n 40)

################################################################################
.PHONY:
clean:    FORCE
	rm -fr $(COBJS)
	rm -f *.gcov *.gcda *.gcno

################################################################################
.PHONY:
tgz:    FORCE
	@mkdir -p tgz
	@tar cvzf tgz/gg.$(DATETIME).tgz $(ARCHIVE)

################################################################################
FORCE:
