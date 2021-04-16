all: fast

get:
	nasm -f elf64 get.asm
	mv get.o src

hashing:
	nasm -f elf64 hashing.asm
	mv hashing.o src

fast: get hashing
	g++ -O0 -o main main.cpp src/hashing.o src/get.o

slow:
	g++ -O0 -o main slow.cpp 
