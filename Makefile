all: grubchess

grubchess: grubchess.c ai.c hashtable.c
	gcc -std=c11 -O4 -g grubchess.c ai.c hashtable.c -o grubchess

test: grubchess
	./grubchess
clean:
	rm -f grubchess
