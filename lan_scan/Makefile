CC = gcc
CFLAGS = -Wall 
LDFLAGS = -lpthread


APP_NAME = lan_scan

SRC = lan_scan.c
OBJ = $(SRC:.c=.o)


all: $(APP_NAME)

$(APP_NAME): $(OBJ)
	$(CC) $(LDFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(APP_NAME) $(OBJ)

.PHONY: all clean
