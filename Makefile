.PHONY: all clean

all:
	$(MAKE) -C src
	$(MAKE) -C unix

clean:
	$(MAKE) -C src clean
	$(MAKE) -C unix clean

