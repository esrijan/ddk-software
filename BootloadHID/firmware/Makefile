# Name: Makefile
# Project: bootloadHID
# Author: Christian Starkjohann
# Creation Date: 2007-03-19
# Tabsize: 4
# Copyright: (c) 2007 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt)
# This Revision: $Id: Makefile 788 2010-05-30 20:54:41Z cs $

###############################################################################
# Configure the following variables according to your AVR. The example below
# is for an ATMega8. Program the device with
#     make fuse    # to set the clock generator, boot section size etc.
#     make flash   # to load the boot loader into flash
#     make lock    # to protect the boot loader from overwriting
#
# Fuse high byte for ATMega8:
# 0xc0 = 1 1 0 0   0 0 0 0 <-- BOOTRST (boot reset vector at 0x1800)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- RSTDISBL (reset pin is enabled)
# Fuse low byte for ATMega8:
# 0x9f = 1 0 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (BrownOut Detector enabled)
#        +-------------------- BODLEVEL (2.7V)
#
###############################################################################

DEVICE = atmega32
#DEVICE = atmega16
#TEST_MODE = 1
DEBUG_LEVEL = 0
ifdef TEST_MODE
BOOTLOADER_ADDRESS = 0x0000
endif
ifeq (${DEVICE},atmega32)
ifndef TEST_MODE
BOOTLOADER_ADDRESS = 0x7000
FUSEH = 0x88
else
FUSEH = 0x89
endif
FUSEL = 0xFF
LOCK = 0x0F
ifndef TEST_MODE
ifndef DEBUG_LEVEL
FWB_ADDRESS = 0x7780
else
ifeq (${DEBUG_LEVEL},0)
FWB_ADDRESS = 0x7780
else
FWB_ADDRESS = 0x7F80
endif
endif
endif
else # atmega16
ifndef TEST_MODE
BOOTLOADER_ADDRESS = 0x3800
FUSEH = 0x88
else
FUSEH = 0x89
endif
FUSEL = 0xFF
LOCK = 0x0F
endif
F_CPU = 16000000

# Using 5V USB2Serial cable directly
#PROGRAMMER := ponyser # For uC w/ active low RESET (working in earlier Hackathons) & for active high (working now)
#PROGRAMMER := ponyseri # For uC w/ active high RESET (working in earlier Hackathon) & for active low (working now)
# Using any serial cable thru MAX232
#PROGRAMMER := max232 # For any uC
# Using an USB ASP programmer
PROGRAMMER := usbasp
#SERIAL_PORT := ttyUSB0 # For USB2Serial
#SERIAL_PORT := ttyS0 # For direct serial
#AVRDUDEFLAGS := -c ${PROGRAMMER} -P /dev/${SERIAL_PORT} -p ${DEVICE}
AVRDUDEFLAGS := -c ${PROGRAMMER} -p ${DEVICE}

AVRDUDE = avrdude ${AVRDUDEFLAGS}

# Omit -fno-* options when using gcc 3, it does not support them.
CFLAGS = -Wall -Os -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions -Iusbdrv -I. -mmcu=${DEVICE} -DF_CPU=${F_CPU} -DBL_ADDR=${BOOTLOADER_ADDRESS} -DDEBUG_LEVEL=${DEBUG_LEVEL}
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.
ifndef TEST_MODE
CFLAGS += -DFWB
ifdef FWB_ADDRESS
CFLAGS += -DFWB_ADDRESS=${FWB_ADDRESS}
endif
else
CFLAGS += -DTEST_MODE
endif

LDFLAGS += -Wl,--relax
LDFLAGS += -Wl,--section-start=.text=${BOOTLOADER_ADDRESS}
ifdef FWB_ADDRESS
LDFLAGS += -Wl,--section-start=.fwb=${FWB_ADDRESS}
endif
LDFLAGS += -Wl,-Map=main.map,--cref

COMPILE = avr-gcc ${CFLAGS}

OBJECTS = usbdrv/usbdrvasm.o usbdrv/oddebug.o fwb.o main.o

# symbolic targets:
all:	main.hex

.c.o:
	${COMPILE} -c $< -o $@

.S.o:
	${COMPILE} -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	${COMPILE} -S $< -o $@

prepare_ddk: main.hex
	${AVRDUDE} -U flash:w:$<:i -U hfuse:w:${FUSEH}:m -U lfuse:w:${FUSEL}:m -U lock:w:${LOCK}:m
	@echo "Check: hfuse (${FUSEH}); lfuse (${FUSEL}); lock (${LOCK})"
	@echo "Possibly need to do a 'write lfuse 0 ${FUSEL}'"
	@echo "Open & Close the ZIF socket & then Do 'make shell' for that"

flash:	all
	${AVRDUDE} -U flash:w:main.hex:i

readflash:
	${AVRDUDE} -U flash:r:read.hex:i

fuse:
ifndef FUSEE
	${AVRDUDE} -U hfuse:w:${FUSEH}:m -U lfuse:w:${FUSEL}:m
else
	${AVRDUDE} -U efuse:w:${FUSEE}:m -U hfuse:w:${FUSEH}:m -U lfuse:w:${FUSEL}:m
endif

lock:
	${AVRDUDE} -U lock:w:${LOCK}:m

read_fuses:
	${UISP} --rd_fuses

shell:
	${AVRDUDE} -v -t

clean:
	rm -f main.hex main.bin main.map *.o usbdrv/*.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
main.bin:	${OBJECTS}
	${COMPILE} -o main.bin ${OBJECTS} ${LDFLAGS}

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -R .eeprom -O ihex main.bin main.hex
	#avr-objcopy -j .text -j .data -O ihex main.bin main.hex
	avr-size main.hex

disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	${COMPILE} -E main.c
