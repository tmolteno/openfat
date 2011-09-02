TARGET = libopenfat.a

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

CFLAGS += -g3 -MD -Wall -Wextra -std=gnu99

SRC = fat.c direntry.c

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(AR) crv $@ $^

.PHONY: clean

clean:	
	-rm -rf *.o *.d $(TARGET)

-include *.d

