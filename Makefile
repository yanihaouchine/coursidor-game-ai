GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 -I$(GSL_PATH)/include -Isrc
LDFLAGS = -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib
LATEX = pdflatex

all: build

%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@ #--coverage

%.o: test/%.c
	gcc $(CFLAGS) -c $< -o $@ --coverage

build: server client
yani.so: yani.o opt.o choosen_move.o state.o imaginary_player.o board.o dijkstra.o
	gcc $(CFLAGS) -shared $^ -o ./install/$@
steven.so: steven.o board.o
	gcc $(CFLAGS) -shared $^ -o ./install/$@
better_tristan.so: better_tristan.o board.o
	gcc $(CFLAGS) -shared $^ -o ./install/$@
tristan.so: tristan.o dijkstra.o board.o
	gcc $(CFLAGS) -shared $^ -o ./install/$@

client: yani.so tristan.so

server: server.o board.o dijkstra.o
	gcc $(CFLAGS) $^ $(LDFLAGS) -o ./install/$@

alltests: alltests.o board.o dijkstra.o
	gcc $(CFLAGS) $^ $(LDFLAGS) -o ./install/$@ -lgcov
	./install/$@

test: alltests

install: server client test

clang:
	find src/*.[ch] | xargs clang-format -i
	find test/*.[ch] | xargs clang-format -i

# Neato

DOT_FILES := $(wildcard *.dot)
PDF_FILES := $(DOT_FILES:.dot=.pdf)

neato: $(PDF_FILES)
	pdftk test_tristan*.pdf cat output test_tristan.pdf
	pdftk test_steven*.pdf cat output test_steven.pdf

%.pdf: %.dot
	neato $< -Tpdf -o $@


TEXSRC = report/main.tex report/parts/*.tex

main.pdf: $(TEXSRC)
	$(LATEX) report/main.tex
	$(LATEX) report/main.tex

report: main.pdf
	evince main.pdf &

clean:
	@rm -f *~ src/*~ test/*~ install/* *.o *.gcda *.gcno *.pdf *.dot *.gcov
	rm -f *.aux *.log *.out *.pdf *.toc	


.PHONY: report client install test clean clang-format neato
