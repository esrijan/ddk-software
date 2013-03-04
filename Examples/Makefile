#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

DDKSW_BASE := ..

USE_CLCD := 1
ifdef USE_CLCD
	CLCD_SRC := clcd.c
else
	CLCD_SRC :=
endif

SRCS := example.c
#SRCS := switch_n_led.c
#SRCS := ssd.c # Currently tested for ATmega168 only. TODO: Test for ATmega32
#SRCS := common.c rgb_pwm.c # Dependent on ATmega168. TODO: Add ATmega32 support; See file for others
#SRCS := pgm_space_storage.c
#SRCS := stepper_motor.c
#SRCS := serial.c serial_demo.c
#SRCS := clcd.c lcd_demo.c
#SRCS := clcd.c ldr_adc.c
#SRCS := serial.c serial_int.c
#SRCS := clcd.c serial.c clcd_serial.c
#SRCS := fan.c
#SRCS := buzzer.c
#SRCS := dac.c serial.c # Not yet tested for its working
#SRCS := clcd.c temp_adc.c
#SRCS := led_cube.c
#SRCS := flash.c serial.c flash_demo.c # Need to try by putting into boot loader &/or NRWW section
#SRCS := ${CLCD_SRC} serial_programmer.c serial_pgmer_demo.c
#SRCS := ${CLCD_SRC} jtag_programmer.c jtag_pgmer_demo.c
#SRCS := ${CLCD_SRC} parallel_programmer.c parallel_pgmer_demo.c # Not yet able to test
#SRCS := clcd.c hex_keypad.c
#SRCS := clcd.c serial.c ask_rf_serial.c # Yet to be tried w/ < 1K baud rate
#SRCS := ask_rf_test.c
#SRCS := ask_rf_test2.c
#SRCS := serial.c ask_rf_demo.c
#SRCS := serial.c ask_rf_demo2.c
#SRCS := waveforms.c
OBJS := $(SRCS:.c=.o)
TARGET := download

#CHIP_NO := 168
#CHIP_NO := 32
CHIP_NO := 16
#CHIP_NO := 8
#CHIP_NO := 128rfa1
#F_CPU := 1000000
#F_CPU := 3686400
#F_CPU := 7372800
#F_CPU := 8000000
F_CPU := 16000000
#F_CPU := 20000000
include ${DDKSW_BASE}/rules.mk
ifdef USE_CLCD
CFLAGS += -DUSE_CLCD
endif
