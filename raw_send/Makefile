CC = gcc
CFLAGS = -Wall

all: raw_send

raw_send: raw_send.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f raw_send raw_send.o
