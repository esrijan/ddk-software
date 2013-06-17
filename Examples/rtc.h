/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Header for RTC DS1307 library.
 *
 * AVR as I2C master, operating on the RTC DS1307 connected as I2C slave.
 */

#ifndef RTC_H
#define RTC_H

typedef enum
{
	hz_1,
	hz_4096,
	hz_8192,
	hz_32768
} Frequency;

void rtc_init(void);
int rtc_set(uint8_t y, uint8_t mo, uint8_t d, uint8_t dy, uint8_t h, uint8_t m, uint8_t s);
int rtc_get(uint8_t *is_halted, uint8_t *y, uint8_t *mo, uint8_t *d, uint8_t *dy, uint8_t *h, uint8_t *m, uint8_t *s);
int rtc_get_str(uint8_t *is_halted, char date_str[15], char time_str[9]);
/* rtc_get_str() format example: date_str[] = "01.06.2075 Sat"; time_str[] = "03:07:00"; */

int rtc_set_sq_wave(Frequency f);
int rtc_reset_sq_wave(void);
int rtc_get_sq_wave(int *enabled, Frequency *f);
#endif
