CC = gcc

all: sigmon test

sigmon: sigmon.c
	$(CC) $< -Wall -o $@

test: test.c
	$(CC) $< -Wall -o $@

clean:
	rm -f sigmon test
