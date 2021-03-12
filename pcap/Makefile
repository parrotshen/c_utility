CC = gcc

all: pcap_gen pcapng_to_pcap

pcap_gen: pcap_gen.o util.o
	$(CC) $^ -o $@

pcapng_to_pcap: pcapng_to_pcap.o util.o
	$(CC) $^ -o $@

%.o: %.c util.h
	$(CC) -Wall -c $<

clean:
	rm -f pcap_gen pcapng_to_pcap *.o
