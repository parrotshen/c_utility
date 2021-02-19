CC = gcc
CFLAGS = -Wall

APPS = multicast_send multicast_recv multicast_group

all: $(APPS)

multicast_send: multicast_send.o
	$(CC) $(CFLAGS) $< -o $@

multicast_recv: multicast_recv.o
	$(CC) $(CFLAGS) $< -o $@

multicast_group: multicast_group.o
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	\rm $(APPS) *.o
