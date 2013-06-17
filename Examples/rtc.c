/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * RTC DS1307 library.
 *
 * AVR as I2C master, operating on the RTC DS1307 connected as I2C slave.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "twi.h"
#include "rtc.h"

#define RTC_DEV_ADDR 0b1101000

void rtc_init(void)
{
	twi_init(standard);
}

int rtc_set(uint8_t y, uint8_t mo, uint8_t d, uint8_t dy, uint8_t h, uint8_t m, uint8_t s)
{
	uint8_t rtc_data[8] = { /* RTC Registers' start address */ 0x00, };
	uint8_t *rtc_reg = rtc_data + 1;

	/* RTC Registers */
	rtc_reg[0] = ((s / 10) << 4) | (s % 10); /* seconds in BCD */
	rtc_reg[1] = ((m / 10) << 4) | (m % 10); /* minutes in BCD */
	rtc_reg[2] = ((h / 10) << 4) | (h % 10); /* hours in BCD */
	rtc_reg[3] = dy; /* day in BCD : 7 for Saturday */
	rtc_reg[4] = ((d / 10) << 4) | (d % 10); /* date in BCD */
	rtc_reg[5] = ((mo / 10) << 4) | (mo % 10); /* month in BCD */
	rtc_reg[6] = ((y / 10) << 4) | (y % 10); /* year in BCD */

	return twi_master_tx(RTC_DEV_ADDR, rtc_data, sizeof(rtc_data));
}

int rtc_get(uint8_t *is_halted, uint8_t *y, uint8_t *mo, uint8_t *d, uint8_t *dy, uint8_t *h, uint8_t *m, uint8_t *s)
{
	int ret;
	uint8_t rtc_data = 0x00; /* RTC Registers' start address */
	uint8_t rtc_reg[7];

	if ((ret = twi_master_tx_rx(RTC_DEV_ADDR, &rtc_data, 1, rtc_reg, 7)) == 0)
	{
		*is_halted = rtc_reg[0] >> 7;
		*s = 10 * ((rtc_reg[0] >> 4) & 0x7) + (rtc_reg[0] & 0xF); /* seconds from BCD */
		*m = 10 * (rtc_reg[1] >> 4) + (rtc_reg[1] & 0xF); /* minutes from BCD */
		*h = 10 * (rtc_reg[2] >> 4) + (rtc_reg[2] & 0xF); /* hours from BCD */
		*dy = rtc_reg[3]; /* day from BCD : 7 for Saturday */
		*d = 10 * (rtc_reg[4] >> 4) + (rtc_reg[4] & 0xF); /* date from BCD */
		*mo = 10 * (rtc_reg[5] >> 4) + (rtc_reg[5] & 0xF); /* month from BCD */
		*y = 10 * (rtc_reg[6] >> 4) + (rtc_reg[6] & 0xF); /* year from BCD */
	}
	return ret;
}

int rtc_get_str(uint8_t *is_halted, char date_str[15], char time_str[9])
/* Format Example: date_str[] = "01.06.2075 Sat"; time_str[] = "03:07:00"; */
{
	int ret;
	uint8_t rtc_data = 0x00; /* RTC Registers' start address */
	uint8_t rtc_reg[7];
	uint8_t day;
	char *day_str[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }; 

	if ((ret = twi_master_tx_rx(RTC_DEV_ADDR, &rtc_data, 1, rtc_reg, 7)) == 0)
	{
		*is_halted = rtc_reg[0] >> 7;
		date_str[0] = '0' + (rtc_reg[4] >> 4);
		date_str[1] = '0' + (rtc_reg[4] & 0xF);
		time_str[2] = '.';
		date_str[3] = '0' + (rtc_reg[5] >> 4);
		date_str[4] = '0' + (rtc_reg[5] & 0xF);
		time_str[5] = '.';
		time_str[6] = '2';
		time_str[7] = '0';
		date_str[8] = '0' + (rtc_reg[6] >> 4);
		date_str[9] = '0' + (rtc_reg[6] & 0xF);
		time_str[10] = ' ';

		day = rtc_reg[3] & 0xF;
		date_str[11] = day_str[day][0];
		date_str[12] = day_str[day][1];
		date_str[13] = day_str[day][2];
		date_str[14] = 0;

		time_str[0] = '0' + (rtc_reg[2] >> 4);
		time_str[1] = '0' + (rtc_reg[2] & 0xF);
		time_str[2] = ':';
		time_str[3] = '0' + (rtc_reg[1] >> 4);
		time_str[4] = '0' + (rtc_reg[1] & 0xF);
		time_str[5] = ':';
		time_str[6] = '0' + ((rtc_reg[0] >> 4) & 0x7);
		time_str[7] = '0' + (rtc_reg[0] & 0xF);
		time_str[8] = 0;
	}
	return ret;
}

int rtc_set_sq_wave(Frequency f)
{
	uint8_t rtc_data[2] = { /* RTC Control Register's start address */ 0x07, };

	rtc_data[1] = 0x10 | (f & 0x3);

	return twi_master_tx(RTC_DEV_ADDR, rtc_data, sizeof(rtc_data));
}

int rtc_reset_sq_wave(void)
{
	uint8_t rtc_data[2] = { /* RTC Control Register's start address */ 0x07, 0x00};

	return twi_master_tx(RTC_DEV_ADDR, rtc_data, sizeof(rtc_data));
}

int rtc_get_sq_wave(int *enabled, Frequency *f)
{
	int ret;
	uint8_t rtc_data = 0x07; /* RTC Control Register's start address */
	uint8_t rtc_reg;

	if ((ret = twi_master_tx_rx(RTC_DEV_ADDR, &rtc_data, 1, &rtc_reg, 1)) == 0)
	{
		*enabled = !!(rtc_reg & 0x10);
		if (*enabled)
			*f = rtc_reg & 0x3;
	}
	return ret;
}
