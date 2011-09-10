.PHONY: images

CFLAGS = -Wall -Wextra -std=gnu99 -g3 -MD -I../src
LDFLAGS = -L../src
LIBS = -lopenfat

SRC = 	fattest.c \
	blockdev_file.c \

OBJ = $(SRC:.c=.o)

fattest: $(OBJ) ../src/libopenfat.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

images: fat12.img fat16.img fat32.img

fat12.img:
	dd if=/dev/zero of=$@ bs=512 count=2048
	mkfs -t msdos $@

fat16.img:
	dd if=/dev/zero of=$@ bs=512 count=20480
	mkfs -t msdos $@

fat32.img:
	dd if=/dev/zero of=$@ bs=512 count=81920
	mkfs -t msdos -F 32 $@

.PHONY: clean

clean:
	-rm -rf *.o *.d fattest

-include *.d

