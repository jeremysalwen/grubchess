all: grubchess

grubchess: grubchess.c ai.c hashtable.c
	gcc -std=c11 -O3 -g -pg grubchess.c ai.c hashtable.c -o grubchess

test: grubchess
	./grubchess
