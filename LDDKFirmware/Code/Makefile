#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

DDKSW_BASE := ../..

#USE_CLCD := 1

FW_VER := 2.2

CHIP_NO := 32
#CHIP_NO := 16
F_CPU := 16000000

EEPROM_START := 0
ifdef USE_CLCD
FLASH_START := 0x2000
else
FLASH_START := 0x1000
endif
ifeq (${CHIP_NO}, 32)
	EEPROM_SIZE := 0x400
	# 0x8000 - 0x1000 /* Boot loader size */
	FLASH_SIZE := 0x7000
else
ifeq (${CHIP_NO}, 16)
	EEPROM_SIZE := 0x200
	# 0x4000 - 0x0800 /* Boot loader size */
	FLASH_SIZE := 0x3800
endif
endif

CSRCS := $(wildcard *.c)
ifndef USE_CLCD
CSRCS := $(filter-out clcd.c, ${CSRCS})
endif
ASRCS := $(wildcard *.S)
OBJS := $(CSRCS:.c=.o) $(ASRCS:.S=.o)
TARGET := usbdev

all: ${TARGET}.hex
#all: ${TARGET}.hex ${TARGET}.eep ${TARGET}.sym ${TARGET}.lss

include ${DDKSW_BASE}/rules.mk

ifdef USE_CLCD
CFLAGS += -DUSE_CLCD
endif
CFLAGS += -DFW_VER=\"${FW_VER}\"
CFLAGS += -DEEPROM_START=${EEPROM_START} -DFLASH_START=${FLASH_START}
CFLAGS += -DEEPROM_SIZE=${EEPROM_SIZE} -DFLASH_SIZE=${FLASH_SIZE}
CFLAGS += -DFWB_ADDR=0x7780
CFLAGS += -DDEBUG_LEVEL=0
LDFLAGS += -L${TOOLS_BASE}/AVR/avr/lib/avr5 # Needed for EEPROM functions

${TARGET}.elf: ${OBJS}

prepare: package

package: uf.tgz
	FW_VER=${FW_VER} ./create_package $^ upgrade_firmware
	${RM} $^

uf.tgz: bootloadHID ${TARGET}.hex
	tar -zcvf $@ $^ > /dev/null

clean:
	${RM} ${TARGET}.hex ${TARGET}.eep ${TARGET}.elf
	${RM} ${TARGET}.lss ${TARGET}.sym ${TARGET}.map
	${RM} *.o *.lst

mrproper: allclean
	${RM} upgrade_firmware
