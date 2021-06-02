CC = gcc

all: tcp_echo tcp_client

tcp_echo: tcp_echo.c
	$(CC) -Wall tcp_echo.c -o tcp_echo

tcp_client: tcp_client.c
	$(CC) -Wall tcp_client.c -o tcp_client

clean:
	rm -f tcp_echo tcp_client
