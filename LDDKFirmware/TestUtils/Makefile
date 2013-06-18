#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

SRCS := $(wildcard *.c)
EXES := $(SRCS:.c=)
DIRS := USBDeviceTest USBLEDTest
DIRS_BUILD := $(DIRS:=_build)
DIRS_CLEAN := $(DIRS:=_clean)

all: ${EXES} ${DIRS_BUILD}

%_build:
	${MAKE} -C $*

clean: ${DIRS_CLEAN}
	${RM} ${EXES}

%_clean:
	${MAKE} -C $* clean
