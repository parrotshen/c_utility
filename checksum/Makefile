CC = gcc
APPS = crc32_csum md5_csum


all: $(APPS)


crc32_csum: crc32_csum.o crc32.o
	$(CC) $^ -o $@

crc32_csum.o: crc32_csum.c crc32.h
	$(CC) -Wall -c $<

crc32.o: crc32.c crc32.h
	$(CC) -Wall -c $<


md5_csum: md5_csum.o md5.o
	$(CC) $^ -o $@

md5_csum.o: md5_csum.c md5.h
	$(CC) -Wall -c $<

md5.o: md5.c md5.h
	$(CC) -Wall -c $<


clean:
	rm -f $(APPS) *.o

.PHONY: all clean
