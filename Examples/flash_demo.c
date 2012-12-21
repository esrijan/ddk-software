/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Flash Operations over Serial CLI
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <ctype.h>

#include "serial.h"
#include "flash.h"

#define FLASH_OPS_ADDR (uint8_t *)(0x1000)
#define FLASH_OPS_LEN SPM_PAGESIZE

void init_serial(void)
{
	usart_init(9600);
}
void byte_to_hex(uint8_t b, char h[3])
{
	h[0] = (b >> 4) & 0x0F;
	h[1] = b & 0x0F;
	h[2] = 0;
	h[0] += (h[0] < 10) ? '0' : ('A' - 10);
	h[1] += (h[1] < 10) ? '0' : ('A' - 10);
}
void hex_to_byte(char h[3], uint8_t *b)
{
	if ((h[0] >= '0') && (h[0] <= '9'))
	{
		*b = (h[0] - '0') << 8;
	}
	else if ((h[0] >= 'A') && (h[0] <= 'F'))
	{
		*b = (h[0] - 'A' + 10) << 8;
	}
	else if ((h[0] >= 'a') && (h[0] <= 'f'))
	{
		*b = (h[0] - 'a' + 10) << 8;
	}
	else
	{
		*b = 0;
		return;
	}
	if (h[1] == 0)
	{
		*b >>= 8;
		return;
	}
	if ((h[1] >= '0') && (h[1] <= '9'))
	{
		*b |= (h[1] - '0');
	}
	else if ((h[1] >= 'A') && (h[1] <= 'F'))
	{
		*b |= (h[1] - 'A' + 10);
	}
	else if ((h[1] >= 'a') && (h[1] <= 'f'))
	{
		*b |= (h[1] - 'a' + 10);
	}
	else
	{
		*b = 0;
		return;
	}
}
void display_help(void)
{
	usart_tx("?: show help\r\n");
	usart_tx("h: show help\r\n");
	usart_tx("r: read flash\r\n");
	usart_tx("w <hex_val>: write flash\r\n");
}
void read_n_display_flash(uint8_t *start_addr, int len)
{
	int off;
	char ascii[3];
	uint16_t word;

	for (off = 0; off < len; off += 2)
	{
		if (!(off & 0xF))
		{
			byte_to_hex((uint8_t)(off), ascii);
			usart_tx(ascii);
			usart_byte_tx(':');
		}
		word = flash_read_word((uint16_t *)(start_addr + off));
		usart_byte_tx(' ');
		byte_to_hex((uint8_t)(word >> 8), ascii);
		usart_tx(ascii);
		usart_byte_tx(' ');
		byte_to_hex((uint8_t)(word & 0xFF), ascii);
		usart_tx(ascii);
		if (!((off + 2) & 0xF))
		{
			usart_tx("\r\n");
		}
	}
}
void write_flash(uint8_t *start_addr, int len, uint8_t val)
{
	int off;
	uint16_t word = (val << 8) | (val ^ 0x1);

	for (off = 0; off < len; off += 2)
	{
		flash_write_word((uint16_t *)(start_addr + off), word);
	}
}

int main(void)
{
	char c;
	char value[3];
	uint8_t val;

	init_serial();

	usart_byte_rx(); // Waiting for a character, typically an <Enter>
	usart_tx("Welcome to eSrijan's Flash CLI\r\n");

	while (1)
	{
		usart_tx("eSrijan> ");
		c = usart_byte_rx();
		usart_byte_tx(c);
		switch (c)
		{
			case 'h':
			case '?':
				usart_tx("\r\n");
				display_help();
				break;
			case 'r':
				usart_tx("\r\n");
				read_n_display_flash(FLASH_OPS_ADDR, FLASH_OPS_LEN);
				break;
			case 'w':
				if ((c = usart_byte_rx()) != ' ')
				{
					usart_tx("\r\nInvalid/Incomplete option\r\n");
					break;
				}
				usart_byte_tx(c);
				while ((c = usart_byte_rx()) == ' ')
				{
					usart_byte_tx(c);
				}
				if (!isxdigit(c))
				{
					usart_tx("\r\nInvalid/Incomplete option\r\n");
					break;
				}
				usart_byte_tx(c);
				value[0] = c;
				c = usart_byte_rx();
				if (c == '\r')
				{
					value[1] = 0;
				}
				else
				{
					if (!isxdigit(c))
					{
						usart_tx("\r\nInvalid/Incomplete option\r\n");
						break;
					}
					usart_byte_tx(c);
					value[1] = c;
					value[2] = 0;
				}
				usart_tx("\r\n");
				hex_to_byte(value, &val);
				write_flash(FLASH_OPS_ADDR, FLASH_OPS_LEN, val);
				read_n_display_flash(FLASH_OPS_ADDR, FLASH_OPS_LEN);
				break;
			default:
				usart_tx("\r\nInvalid option: ");
				usart_byte_tx(c);
				usart_tx("\r\n");
				display_help();
				break;
		}
	}

	return 0;
}
