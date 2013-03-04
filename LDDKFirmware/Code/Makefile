#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

BOOTLOADHID_BASE := ../..

FW_VER := 1.1
#USB_CLCD := 1
ifdef USB_CLCD
CSRCS := $(wildcard *.c)
else
CSRCS := $(filter-out clcd.c, $(wildcard *.c))
endif
ASRCS := $(wildcard *.S)
OBJS := $(CSRCS:.c=.o) $(ASRCS:.S=.o)
TARGET := usbdev

#CHIP_NO := 168
#CHIP_NO := 32
CHIP_NO := 16
EEPROM_START := 0
ifdef USB_CLCD
FLASH_START := 0x1000
else
FLASH_START := 0x0400
endif
ifeq (${CHIP_NO}, 32)
	EEPROM_SIZE := 0x400
	# 0x8000 - 0x1000
	FLASH_SIZE := 0x7000
else
ifeq (${CHIP_NO}, 16)
	EEPROM_SIZE := 0x200
	# 0x4000 - 0x0800
	FLASH_SIZE := 0x3800
endif
endif
F_CPU := 16000000
#F_CPU := 20000000
include ../../rules.mk
CFLAGS += -DFW_VER=\"${FW_VER}\"
CFLAGS += -DEEPROM_START=${EEPROM_START} -DFLASH_START=${FLASH_START}
CFLAGS += -DEEPROM_SIZE=${EEPROM_SIZE} -DFLASH_SIZE=${FLASH_SIZE}
CFLAGS += -DMEM_EEPROM
CFLAGS += -DDEBUG_LEVEL=0
ifdef USB_CLCD
CFLAGS += -DUSB_CLCD
endif
LDFLAGS += -L${TOOLS_BASE}/AVR/avr/lib/avr5 # Needed for EEPROM functions

package: uf.tgz
	FW_VER=${FW_VER} ./create_package $^ upgrade_firmware
	${RM} $^

uf.tgz: ${TARGET}.hex
	@${MAKE} -C ${BOOTLOADHID_BASE}/BootloadHID/commandline && cp ${BOOTLOADHID_BASE}/BootloadHID/commandline/bootloadHID .
	tar -zcvf $@ bootloadHID ${TARGET}.hex > /dev/null
	@${RM} bootloadHID
