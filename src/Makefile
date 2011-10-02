TARGET = libopenfat.a

DESTDIR         ?= /usr/local
INCDIR          = $(DESTDIR)/$(PREFIX)/include
LIBDIR          = $(DESTDIR)/$(PREFIX)/lib
INSTALL         = install

CC = $(PREFIX)-gcc
AR = $(PREFIX)-ar

CFLAGS += -g3 -MD -Wall -Wextra -std=gnu99 -I../include \
	-Wno-char-subscripts -Werror

SRC = fat_core.c direntry.c mbr.c write.c unixlike.c

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(AR) crv $@ $^

.PHONY: clean install

clean:	
	-rm -rf *.o *.d $(TARGET)

install: $(TARGET)
	$(INSTALL) -d $(INCDIR)
	$(INSTALL) -d $(INCDIR)/openfat
	$(INSTALL) -d $(LIBDIR)
	$(INSTALL) -m 0644 $(TARGET) $(LIBDIR)
	$(INSTALL) -m 0644 ../include/openfat.h $(INCDIR)
	$(INSTALL) -m 0644 ../include/openfat/* $(INCDIR)/openfat
	
-include *.d

