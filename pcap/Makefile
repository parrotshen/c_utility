CC = gcc

all: pcap_gen

pcap_gen: pcap_gen.o util.o
	$(CC) -Wall $^ -o $@

pcap_gen.o: pcap_gen.c util.h
	$(CC) -Wall -c $<

util.o: util.c util.h
	$(CC) -Wall -c $<

clean:
	rm -f pcap_gen *.o
