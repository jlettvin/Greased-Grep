#!/usr/bin/env make
# Makefile for building lexer
# 

DATETIME=`date +%Y%m%d%H%M%S`

CC=g++
DEFINES=
SHARED_OPTS=-std=c++17 -I. -Wall -Wextra -Werror -fPIC -ggdb
GOPTS=-g -O0 -coverage -pg -fprofile-arcs -ftest-coverage
COPTS=-O3 -Wextra -Wall -Werror -std=c++17
ALLOPTS=$(DEFINES) $(SHARED_OPTS) $(GOPTS)
CPPFLAGS=$(ALLOPTS)
#LFLAGS=-lgcov --coverage -lfmt -lstdc++fs
LFLAGS=-lfmt -lstdc++fs
COBJS=gg
ARCHIVE=.gitignore LICENSE Makefile README.md gg.cpp

EMPTY=
REJECT=-$(EMPTY)m$(EMPTY)n$(EMPTY)o

all: before gg test after

.PHONY:
test: FORCE
	./gg -i -s abc def ghi jkl .
	./gg -i -s +abc +def +ghi +jkl $(REJECT) .

gg:	gg.cpp
	g++ -O3 -Wextra -Wall -Werror -std=c++17 -o gg gg.cpp -lfmt -lstdc++fs

#	$(CC) $(COPTS) -o $@ $< $(LOPTS)
#	@g++ -O3 -Wextra -Wall -Werror -std=c++17 -o gg gg.cpp -lfmt -lstdc++fs
#	@strip gg
#	@./gg -i -s abc def ghi jkl .
#	@./gg -i -s +abc +def +ghi +jkl $(REJECT) .

.PHONY:
before:    FORCE
	@printf "\n\t\t\t\t\tMAKE\n"
	@echo $$(yes '-' | head -n 40)
	@seq 3 | xargs -I -- echo $$(yes 'v' | head -n 40)
	@echo $$(yes '-' | head -n 40)

.PHONY:
after:    FORCE
	@echo $$(yes '-' | head -n 40)
	@seq 3 | xargs -I -- echo $$(yes '^' | head -n 40)
	@echo $$(yes '-' | head -n 40)

.PHONY:
clean:    FORCE
	rm -fr $(COBJS)
	rm -f *.gcov *.gcda *.gcno

.PHONY:
tgz:    FORCE
	@mkdir -p tgz
	@tar cvzf tgz/gg.$(DATETIME).tgz $(ARCHIVE)

FORCE:
