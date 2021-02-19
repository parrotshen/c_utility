AR = ar
CC = gcc
CFLAGS = -Wall
LDFLAGS = 

all: libparser.a libparser.so text hex ipv4 ipv6

libparser.a: libparser.o
	$(AR) rcs $@ $<

libparser.so: libparser.o
	$(CC) -shared $(LDFLAGS) -o $@ $<

text: text.o libparser.o
	$(CC) $(LDFLAGS) -o $@ $^

hex: hex.o libparser.o
	$(CC) $(LDFLAGS) -o $@ $^

ipv4: ipv4.o libparser.o
	$(CC) $(LDFLAGS) -o $@ $^

ipv6: ipv6.o libparser.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c libparser.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f libparser.a libparser.so
	rm -f text hex ipv4 ipv6
	rm -f *.o
