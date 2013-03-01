#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

BOOTLOADHID_BASE := ../..

CSRCS := $(wildcard *.c)
ASRCS := $(wildcard *.S)
OBJS := $(CSRCS:.c=.o) $(ASRCS:.S=.o)
TARGET := usbdev

#CHIP_NO := 168
#CHIP_NO := 32
CHIP_NO := 16
F_CPU := 16000000
#F_CPU := 20000000
include ../../rules.mk
CFLAGS += -DDEBUG_LEVEL=0
LDFLAGS += -L${TOOLS_BASE}/AVR/avr/lib/avr5 # Needed for EEPROM functions
