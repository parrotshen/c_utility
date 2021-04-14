CC = gcc

all: addrmap memsize

addrmap: addrmap.c
	$(CC) -Wall $^ -o $@

memsize: memsize.c
	$(CC) -Wall $^ -o $@

clean:
	rm -f addrmap memsize
