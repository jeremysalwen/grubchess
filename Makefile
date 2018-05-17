all: grubchess

grubchess: grubchess.c ai.c
	gcc -std=c11 -O3 -g grubchess.c ai.c -o grubchess

test: grubchess
	./grubchess
