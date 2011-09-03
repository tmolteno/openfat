ifndef HOST
HOST = unix
endif

ifeq ($(HOST),stm32)
CROSS_COMPILE ?= arm-none-eabi-
CFLAGS += -mcpu=cortex-m3 -mthumb
LDFLAGS += -mcpu=cortex-m3 -mthumb -march=armv7 -mfix-cortex-m3-ldrd -msoft-float 
endif

export CROSS_COMPILE
export CFLAGS
export LDFLAGS

.PHONY: all clean

all:
	$(MAKE) -C src
	$(MAKE) -C $(HOST)

clean:
	$(MAKE) -C src clean
	$(MAKE) -C $(HOST) clean

