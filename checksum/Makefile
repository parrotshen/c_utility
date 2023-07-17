APPS = crc16_csum crc32_csum md5_csum ip_csum tcp_csum udp_csum icmp_csum

all:
	@gcc -Wall -c onec.c
	@gcc -Wall -c parser.c
	@for i in $(APPS) ; do \
        make -f Makefile.$$i ; \
    done

clean:
	@for i in $(APPS) ; do \
        make -f Makefile.$$i clean ; \
    done
	@rm -f onec.o parser.o

.PHONY: all clean
