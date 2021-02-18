CC = gcc
CFLAGS = -Wall
LDFLAGS = 


APP_NAME = sos


#################
#  Source
#################

SRC += sos_main.c 
SRC += sos_common.c
SRC += sos_sip.c


DEP_CFLAGS = $(filter-out -g, $(CFLAGS))
DEPEND_CMD = $(CC) $(DEP_CFLAGS) -M $<

OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.c.d)


#################
#  Build
#################

all: $(APP_NAME)
	@echo "Build ... $(APP_NAME)"

-include $(DEP)

$(APP_NAME): $(OBJ)
	@$(CC) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

ifneq ($(MAKECMDGOALS), clean)
%.c.d: %.c
	@echo Generating depend file $@
	@echo "$@ \\"        >  $@
	@echo -n "$(dir $<)" >> $@
	@$(DEPEND_CMD)       >> $@ || rm $@
endif


clean:
	@echo "Clean ... $(APP_NAME)"
	@rm $(APP_NAME) $(OBJ) $(DEP)

.PHONY: $(APP_NAME) clean
