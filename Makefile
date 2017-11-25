EMPTY=
REJECT=-$(EMPTY)m$(EMPTY)n$(EMPTY)o

all:
	@g++ -O3 -Wextra -Wall -Werror -std=c++17 -o gg gg.cpp -lfmt -lstdc++fs
	@strip gg
	@./gg -i -s abc def ghi jkl .
	@./gg -i -s +abc +def +ghi +jkl $(REJECT) .

