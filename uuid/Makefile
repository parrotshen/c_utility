CC = gcc

all: utest

utest: utest.o uuid.o sysdep.o
	$(CC) -lcrypto $^ -o $@

%.o: %.c global.h sysdep.h uuid.h
	$(CC) -Wall -c $<

clean:
	rm -f utest *.o
	rm -f nodeid state
