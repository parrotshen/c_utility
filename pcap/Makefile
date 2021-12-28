CC = gcc

all: pcap_gen pcap_dlt_user pcapng_to_pcap

pcap_gen: pcap_gen.o util.o
	$(CC) $^ -o $@

pcap_dlt_user: pcap_dlt_user.o util.o
	$(CC) $^ -o $@

pcapng_to_pcap: pcapng_to_pcap.o util.o
	$(CC) $^ -o $@

%.o: %.c util.h
	$(CC) -Wall -c $<

clean:
	rm -f pcap_gen pcap_dlt_user pcapng_to_pcap *.o
