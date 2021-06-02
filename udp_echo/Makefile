CC = gcc

all: udp_echo udp_test

udp_echo: udp_echo.c
	$(CC) -Wall udp_echo.c -o udp_echo

udp_test: udp_test.c
	$(CC) -Wall udp_test.c -o udp_test

clean:
	rm -f udp_echo udp_test
