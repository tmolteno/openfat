CROSS_COMPILE ?= arm-none-eabi-

CC = $(CROSS_COMPILE)gcc

CFLAGS += -Wall -Wextra -std=gnu99 -g3 -MD -I../include -DSTM32F1
LDFLAGS += -L../src \
	-nostartfiles -Wl,-T,stm32.ld -Wl,--defsym,_stack=0x20005000 
LIBS = -lopenfat -lopencm3_stm32f1 -lc -lnosys

SRC = example.c mmc.c

OBJ = $(SRC:.c=.o)

example.elf: $(OBJ) ../src/libopenfat.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY:	clean install

clean:
	-rm -rf *.o *.d example.elf

-include *.d

