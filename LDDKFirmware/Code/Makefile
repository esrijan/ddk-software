#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

DDKSW_BASE := ../..

FW_VER := 1.2
#USE_CLCD := 1
#USE_FLASH := 1
CSRCS := $(wildcard *.c)
ifndef USE_CLCD
CSRCS := $(filter-out clcd.c, ${CSRCS})
endif
ifndef USE_FLASH
CSRCS := $(filter-out flash.c, ${CSRCS})
endif
ASRCS := $(wildcard *.S)
OBJS := $(CSRCS:.c=.o) $(ASRCS:.S=.o)
TARGET := usbdev

#CHIP_NO := 168
#CHIP_NO := 32
CHIP_NO := 16
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
F_CPU := 16000000
#F_CPU := 20000000
include ${DDKSW_BASE}/rules.mk
CFLAGS += -DFW_VER=\"${FW_VER}\"
CFLAGS += -DEEPROM_START=${EEPROM_START} -DFLASH_START=${FLASH_START}
CFLAGS += -DEEPROM_SIZE=${EEPROM_SIZE} -DFLASH_SIZE=${FLASH_SIZE}
ifdef USE_FLASH
CFLAGS += -DMEM_FLASH
else
CFLAGS += -DMEM_EEPROM
endif
CFLAGS += -DDEBUG_LEVEL=0
ifdef USE_CLCD
CFLAGS += -DUSE_CLCD
endif
LDFLAGS += -L${TOOLS_BASE}/AVR/avr/lib/avr5 # Needed for EEPROM functions

prepare: package

package: uf.tgz
	FW_VER=${FW_VER} ./create_package $^ upgrade_firmware
	${RM} $^

uf.tgz: bootloadHID ${TARGET}.hex
	tar -zcvf $@ $^ > /dev/null

mrproper: allclean
	${RM} upgrade_firmware
