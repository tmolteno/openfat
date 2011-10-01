ifndef HOST
HOST = unix
endif

ifeq ($(HOST),stm32)
PREFIX ?= arm-none-eabi
CFLAGS += -mcpu=cortex-m3 -mthumb
LDFLAGS += -mcpu=cortex-m3 -mthumb -march=armv7 -mfix-cortex-m3-ldrd -msoft-float 
endif

export PREFIX
export CFLAGS
export LDFLAGS

.PHONY: all FORCE 

all:
	$(MAKE) -C src
	$(MAKE) -C $(HOST)

%: FORCE
	$(MAKE) -C src $@
	$(MAKE) -C $(HOST) $@

