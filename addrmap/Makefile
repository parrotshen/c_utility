CC = gcc

all: addrmap memsize partition

addrmap: addrmap.c
	$(CC) -Wall $^ -o $@

memsize: memsize.c
	$(CC) -Wall $^ -o $@

partition: partition.c
	$(CC) -Wall $^ -o $@

clean:
	rm -f addrmap memsize partition
