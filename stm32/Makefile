CROSS_COMPILE ?= arm-none-eabi-

CC = $(CROSS_COMPILE)gcc

CFLAGS += -Wall -Wextra -std=gnu99 -g3 -MD -I../src
LDFLAGS += -L../src \
	-nostartfiles -Wl,-T,stm32.ld
LIBS = -lopenfat -lopencm3_stm32 -lc -lnosys

SRC = example.c mmc.c

OBJ = $(SRC:.c=.o)

example.elf: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY:	clean

clean:
	-rm -rf *.o *.d example.elf

-include *.d

