CC = gcc
CFLAGS = -Wall

all: color control ascii

color: color.c
	$(CC) $(CFLAGS) $< -o $@

control: control.c
	$(CC) $(CFLAGS) $< -o $@

ascii: ascii.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f color control ascii
