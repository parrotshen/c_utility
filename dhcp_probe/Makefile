CC = gcc
APP_NAME = dhcprobe

SRC += dhcp_main.c
SRC += dhcp_options.c
SRC += dhcp_packet.c
SRC += dhcp_log.c

CLN_FILES = *.o *.c.d


##################
# CFLAGS
##################

CFLAGS = -Wall


##################
# LDFLAGS
##################

LDFLAGS = 


#################
# Build
#################

DEP_CFLAGS= $(filter-out -g, $(CFLAGS))
DEPEND_CMD= $(CC) $(DEP_CFLAGS) -M $<

OBJ := $(SRC:.c=.o)


all: $(APP_NAME)
	strip $(APP_NAME)

-include $(SRC:.c=.c.d)

$(APP_NAME): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@
	@echo "Build ... $(APP_NAME)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.c.d: %.c
	@echo Generating depend file $(subst ./,,$(@))
	@echo "$@ \\" > $@
	@echo -n "$(dir $<)" >> $@
	@$(DEPEND_CMD) >> $@ || $(RM) $@

config:

clean:
	@rm -f $(APP_NAME) $(CLN_FILES)

.PHONY: all clean
