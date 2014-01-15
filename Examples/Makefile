#
# Copyright (C) eSrijan Innovations Private Limited
#
# Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
#
# Licensed under: JSL (See LICENSE file for details)
#

DDKSW_BASE := ..

USE_CLCD := 1

#DDK_VER ?=
#DDK_VER ?= 1.1
DDK_VER ?= 2.1

#CHIP_NO := 128rfa1
#F_CPU := 16000000
#F_CPU := 20000000
ifeq (${DDK_VER},2.1)
CHIP_NO := 32
F_CPU := 16000000
BL_VER := # Defaults to --v2.1
else
ifeq (${DDK_VER},1.1)
CHIP_NO := 16
F_CPU := 16000000
BL_VER := --v1
else
CHIP_NO := 168
F_CPU := 1000000
#F_CPU := 3686400
#F_CPU := 7372800
#F_CPU := 8000000
BL_VER := # Defaults to --v2.1
endif
endif

ifdef USE_CLCD
	CLCD_OBJ := clcd.o
else
	CLCD_OBJ :=
endif

LIB_SRCS := common.c clcd.c twi.c rtc.c
HEX_TGTS := \
	example.hex \
	pgm_space_storage.hex \
	blink_led.hex \
	switch_n_led.hex \
	lcd_demo.hex \
	stepper_motor.hex \
	ldr_adc.hex \
	temp_adc.hex \
	fan.hex \
	switch_n_ir.hex \
	rtc_clcd.hex
# TODO: Not yet tested on ATmega168: stepper_motor.hex, fan.hex, switch_n_ir.hex, rtc_clcd.hex
# TODO: To be retested: stepper_motor.hex, temp_adc.hex, fan.hex

ifeq (${CHIP_NO},168)

HEX_TGTS += \
	ssd.hex \
	rgb_pwm.hex
# TODO: Modify & Test for ATmega16/32: ssd.hex, rgb_pwm.hex

else
ifeq (${CHIP_NO},$(filter ${CHIP_NO},16 32))

LIB_SRCS += serial.c serial_programmer.c jtag_programmer.c parallel_programmer.c
HEX_TGTS += \
	serial_demo.hex \
	serial_int.hex \
	clcd_serial.hex \
	dac.hex \
	pwm2.hex \
	buzzer.hex \
	waveforms.hex \
	jana_gana_mana.hex \
	hex_keypad.hex \
	led_cube.hex \
	rtc_serial.hex \
	ask_rf_test.hex \
	ask_rf_test2.hex \
	ask_rf_demo.hex \
	ask_rf_demo2.hex \
	ask_rf_serial.hex \
	ask_rf_ind_rx_tx.hex \
	ir_over_serial.hex \
	ir_send_recv.hex \
	ir_remote.hex \
	l293d.hex
# TODO: Not yet completed/tested: dac.hex, pwm2.hex, l293d.hex,
# 	ask_rf_serial.hex (try w/ < 1K baud rate), ask_rf_ind_rx_tx.hex, ir_*

ifneq (${DDK_VER},)

HEX_TGTS += \
	serial_pgmer_demo.hex \
	jtag_pgmer_demo.hex \
	parallel_pgmer_demo.hex
# TODO: Not yet completed/tested: parallel_pgmer_demo.hex (not yet able to test)
# TODO: Remove warnings from *_programmer.c

endif

ifeq (${DDK_VER},2.1)

LIB_SRCS += flash.c
HEX_TGTS += \
	menu.hex \
	led_array.hex \
	clock.hex \
	flash_demo.hex \
	feature_test.hex

endif

endif
endif

build: ${HEX_TGTS}

include ${DDKSW_BASE}/rules.mk

ifdef USE_CLCD
CFLAGS += -DUSE_CLCD
endif
ifneq (${DDK_VER},)
CFLAGS += -DDDK_VER=$(shell echo ${DDK_VER} | cut -d. -f1)
endif
ifeq (${DDK_VER},2.1)
CFLAGS += -DFWB_ADDR=0x7780
endif

example.elf: example.o

pgm_space_storage.elf: pgm_space_storage.o

blink_led.elf: blink_led.o

switch_n_led.elf: switch_n_led.o

ssd.elf: ssd.o

rgb_pwm.elf: common.o rgb_pwm.o

lcd_demo.elf: clcd.o lcd_demo.o

stepper_motor.elf: stepper_motor.o

ldr_adc.elf: clcd.o ldr_adc.o

temp_adc.elf: clcd.o temp_adc.o

fan.elf: fan.o

switch_n_ir.elf: switch_n_ir.o

rtc_clcd.elf: twi.o rtc.o clcd.o rtc_clcd.o

serial_demo.elf: serial.o serial_demo.o

serial_int.elf: serial.o serial_int.o

clcd_serial.elf: clcd.o serial.o clcd_serial.o

dac.elf: serial.o dac.o

pwm2.elf: pwm2.o

buzzer.elf: buzzer.o

waveforms.elf: waveforms.o

jana_gana_mana.elf: jana_gana_mana.o

serial_pgmer_demo.elf: ${CLCD_OBJ} serial_programmer.o serial_pgmer_demo.o

jtag_pgmer_demo.elf: ${CLCD_OBJ} jtag_programmer.o jtag_pgmer_demo.o

parallel_pgmer_demo.elf: ${CLCD_OBJ} parallel_programmer.o parallel_pgmer_demo.o

hex_keypad.elf: clcd.o hex_keypad.o

led_cube.elf: led_cube.o

rtc_serial.elf: twi.o rtc.o serial.o rtc_serial.o

ask_rf_test.elf: ask_rf_test.o

ask_rf_test2.elf: ask_rf_test2.o

ask_rf_demo.elf: serial.o ask_rf_demo.o

ask_rf_demo2.elf: serial.o ask_rf_demo2.o

ask_rf_serial.elf: clcd.o serial.o ask_rf_serial.o

ask_rf_ind_rx_tx.elf: serial.o ask_rf_ind_rx_tx.o

ir_over_serial.elf: serial.o ir_over_serial.o

ir_send_recv.elf: ir_send_recv.o

ir_remote.elf: ir_remote.o

l293d.elf: l293d.o

menu.elf: clcd.o menu.o

led_array.elf: led_array.o

clock.elf: twi.o rtc.o clcd.o clock.o

flash_demo.elf: flash.o serial.o flash_demo.o

feature_test.elf: serial.o twi.o rtc.o clcd.o feature_test.o

clean:
	@echo -n "Basic cleaning ... "
	@${RM} ${HEX_TGTS} $(HEX_TGTS:.hex=.eep) $(HEX_TGTS:.hex=.elf)
	@${RM} $(HEX_TGTS:.hex=.lss) $(HEX_TGTS:.hex=.sym) $(HEX_TGTS:.hex=.map)
	@${RM} *.o *.lst
	@echo "done"

mrproper: allclean
