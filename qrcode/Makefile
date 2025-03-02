CC = gcc

all: qrcode

qrcode: qrcode.o generate.o pattern.o encode.o masking.o format.o reedsolomon.o
	$(CC) $^ -lm -o $@

%.o: %.c generate.h pattern.h encode.h masking.h format.h reedsolomon.h
	$(CC) -Wall -c $<

clean:
	rm -f qrcode *.o
