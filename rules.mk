#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

# Before including this file in your main Makefile, make sure that:
# 1. Either TARGET is defined to the final filename in which the code needs to
# be compiled into, and ${TARGET}.elf: .o's dependencies are defined, Or,
# 	The main Makefile defines its own rules for the target(s)
# 2. In any case, the clean rule has to be defined by the main Makefile
# 2. Optionally, CHIP_NO. Otherwise, default is 168
# 3. Optionally, F_CPU. Otherwise, default is 1000000 (1 MHz)

CROSS_COMPILE ?= avr-

CC := ${CROSS_COMPILE}gcc
AS := ${CROSS_COMPILE}as
LD := ${CROSS_COMPILE}ld
OBJCOPY := ${CROSS_COMPILE}objcopy
OBJDUMP := ${CROSS_COMPILE}objdump
SIZE := ${CROSS_COMPILE}size
NM := ${CROSS_COMPILE}nm
AVRDUDE := avrdude
DDKSW_BASE ?= .

# MCU name
CHIP_NO ?= 168
MCU := atmega${CHIP_NO}
BURN_MCU := m${CHIP_NO}
ifeq (${CHIP_NO}, 168)
# For Fuse settings
# (details in firmware/Makefile of vusb / or the corresponding avr/io*.h files)
EFUSE ?= 0x01
HFUSE ?= 0xDF
LFUSE ?= 0x62
LOCK ?= 0x3F
endif
# Processor frequency.
#     This will define a symbol, F_CPU, in all source code files equal to the 
#     processor frequency. You can then use this symbol in your source code to 
#     calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#     automatically to create a 32-bit value in your source code.
F_CPU ?= 1000000

GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d
CFLAGS = -mmcu=${MCU} -I. ${GENDEPFLAGS}

# Output format. (can be srec, ihex, binary)
FORMAT := ihex
# Optimization level, can be [0, 1, 2, 3, s]. 
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT := s
# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG := dwarf-2
# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS := 
# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD := -std=gnu99
# Place -D or -U options here
CDEFS := -DF_CPU=$(F_CPU)UL
# Place -I options here
CINCS :=
#---------------- Compiler Options ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#CFLAGS += -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -fsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions
CFLAGS += -Wall -Wextra
#CFLAGS += -Wshadow
CFLAGS += -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare -Waggregate-return
#CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused
#CFLAGS += -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)

ASFLAGS = -mmcu=${MCU} -I. -x assembler-with-cpp

# Place -D or -U options here
ASDEFS := -DF_CPU=$(F_CPU)UL
#---------------- Assembler Options ----------------
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
ASFLAGS += $(ASDEFS)
#ASFLAGS += -Wa,-adhlns=$(<:.S=.lst),-gstabs 

LDFLAGS = -mmcu=${MCU}
#---------------- Linker Options ----------------
#  -Wl,...:   tell GCC to pass this to linker.
#  -Map:      create map file
#  --cref:    add cross reference to  map file
LDFLAGS += -Wl,-Map=$(@:.elf=.map),--cref
LDFLAGS += -Wl,--relax,--gc-sections
#LDFLAGS += -Wl,--section-start=.text=0x0

#---------------- Library Options ----------------
# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min
# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt
# If this is left blank, then it will use the Standard printf version.
PRINTF_LIB = 
#PRINTF_LIB = ${PRINTF_LIB_MIN}
#PRINTF_LIB = ${PRINTF_LIB_FLOAT}
# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min
# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt
# If this is left blank, then it will use the Standard scanf version.
SCANF_LIB = 
#SCANF_LIB = ${SCANF_LIB_MIN}
#SCANF_LIB = ${SCANF_LIB_FLOAT}
MATH_LIB = 
#MATH_LIB = -lm
LDFLAGS += ${PRINTF_LIB} ${SCANF_LIB} ${MATH_LIB}
#---------------- External Memory Options ----------------
# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,-Tdata=0x801100,--defsym=__heap_end=0x80ffff
# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff
EXTMEMOPTS =
LDFLAGS += ${EXTMEMOPTS}

# Using 5V USB2Serial cable directly
#PROGRAMMER := ponyser # For uC w/ active low RESET (working in earlier Hackathons) & for active high (working now)
#PROGRAMMER := ponyseri # For uC w/ active high RESET (working in earlier Hackathon) & for active low (working now)
# Using any serial cable thru MAX232
#PROGRAMMER := max232 # For any uC
# Using an USB ASP programmer
PROGRAMMER := usbasp
#SERIAL_PORT := ttyUSB0 # For USB2Serial
#SERIAL_PORT := ttyS0 # For direct serial
#AVRDUDEFLAGS := -c ${PROGRAMMER} -P /dev/${SERIAL_PORT} -p ${BURN_MCU}
AVRDUDEFLAGS := -c ${PROGRAMMER} -p ${BURN_MCU}

ifdef EFUSE
FUSELIST += efuse:w:${EFUSE}:m
endif
ifdef HFUSE
FUSELIST += hfuse:w:${HFUSE}:m
endif
ifdef LFUSE
FUSELIST += lfuse:w:${LFUSE}:m
endif
ifdef FUSE
FUSELIST += fuse:w:${FUSE}:m
endif

download: bootloadHID ${TARGET}.hex
	sudo ./bootloadHID -r ${TARGET}.hex

prepare: bootloadHID

bootloadHID:
	make -C ${DDKSW_BASE}/BootloadHID/commandline
	cp ${DDKSW_BASE}/BootloadHID/commandline/bootloadHID $@
	make -C ${DDKSW_BASE}/BootloadHID/commandline clean

burn: ${TARGET}.hex
	${AVRDUDE} ${AVRDUDEFLAGS} -U flash:w:$<:i

erase:
	${AVRDUDE} ${AVRDUDEFLAGS} -e

read:
	${AVRDUDE} ${AVRDUDEFLAGS} -U flash:r:${TARGET}.rd.hex:i

burnfuse:
	${AVRDUDE} ${AVRDUDEFLAGS} $(addprefix -U , ${FUSELIST})

#readfuse:
#	${AVRDUDE} ${AVRDUDEFLAGS} -U hfuse:r:-:m -U lfuse:r:-:m

lock:
	${AVRDUDE} ${AVRDUDEFLAGS} -U lock:w:${LOCK}:m

shell:
	${AVRDUDE} -v ${AVRDUDEFLAGS} -t

%.hex: %.elf
	${OBJCOPY} -O ${FORMAT} -R .eeprom $< $@
#	${OBJCOPY} -j .text -j .data -O ${FORMAT} $< $@
	${SIZE} $@

%.eep: %.elf
	${OBJCOPY} -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 \
		-O ${FORMAT} $< $@

%.sym: %.elf
	$(NM) -n $< > $@

%.lss: %.elf
	${OBJDUMP} -h -S $< > $@

%.elf:
	${CC} $^ -o $@ ${LDFLAGS}

%.S: %.c
	${CC} -S ${CFLAGS} $^ --output $@

allclean: clean
	${RM} bootloadHID
	${RM} -r .dep

# Include the dependency files.
-include $(shell mkdir -p .dep) $(wildcard .dep/*)
