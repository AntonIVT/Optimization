NASMFLAGS   = -f elf64
CFLAGS      = -O0
MAKEMAIN    = -o main main.cpp
DEFSLOW     = -D SLOW
DEFSPEED    = -D SPEED_TEST
DEFMAINTEST = -D MAIN_TEST
CDEBUGFLAGS = -g  -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr 
SFMLFLAGS   = -lsfml-graphics -lsfml-window -lsfml-system

all: main

get:
	nasm $(NASMFLAGS) get.asm
	mv get.o src

hashing:
	nasm $(NASMFLAGS) hashing.asm
	mv hashing.o src

main: get hashing
	g++ $(CFLAGS) $(MAKEMAIN) src/hashing.o src/get.o

fast: get hashing
	g++ $(CFLAGS) $(MAKEMAIN) $(DEFSPEED) src/hashing.o src/get.o

fast_debug: get hashing
	g++ $(CFLAGS) $(MAKEMAIN) $(CDEBUGFLAGS) $(DEFMAINTEST) src/hashing.o src/get.o

slow:
	g++ $(CFLAGS) $(MAKEMAIN) $(DEFSLOW) $(DEFSPEED)

slow_debug:
	g++ $(CFLAGS) $(MAKEMAIN) $(DEFSLOW) $(CDEBUGFLAGS) $(DEFMAINTEST)

get_plot:
	g++ $(CFLAGS) $(MAKEMAIN) $(DEFSLOW) -D SPEED_TEST_COUNT=30 -D PLOT
	python plot.py
