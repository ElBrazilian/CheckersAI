_GUI = $(if $(NOGUI),, -D GUI)
_DEBUG = $(if $(DEBUG),-D DEBUG,)
_OPT = $(if $(OPT),-O3 -flto,)
CC = gcc
CFLAGS = -g -O3 -gdwarf-4 -std=c99 -Wall $(_OPT) $(_GUI) $(_DEBUG) -I./include
LDFLAGS = -lm

.PHONY: clean doc check-syntax compile-all launch-tests

# rule to remove all .o files and all executables
clean:
	- rm -f *.o
	- rm -f out/*.o
	- find . -maxdepth 1 -executable -type f \( ! -name "Makefile" \) -delete

# compile rules
%.o: %.c
	$(CC) $(CFLAGS) -o out/$@ -c $^
out/%.o: ./src/%.c
	$(CC) $(CFLAGS) -o $@ -c $^


main: out/dames_ai.o out/board.o out/moves.o out/interface.o 
	$(CC) $(CFLAGS) -o out/$@ $^ $(LDFLAGS)

raco:
	raco exe racket/dames-interface.rkt

play-solo:
	./racket/dames-interface - - 

play-second: main
	./racket/dames-interface - ./out/main 

play-solo: main
	./racket/dames-interface ./out/main  - 

ai-battle: main
	./racket/dames-interface ./out/main  ./out/main