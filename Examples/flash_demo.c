/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Flash Operations for DDK v2.1 over Serial CLI.
 * Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <ctype.h>

#include "serial.h"
#include "flash.h"

#define FLASH_OPS_ADDR (uint8_t *)(0x4000)
#define FLASH_OPS_LEN BLOCK_SIZE

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
		*b = (h[0] - '0') << 4;
	}
	else if ((h[0] >= 'A') && (h[0] <= 'F'))
	{
		*b = (h[0] - 'A' + 10) << 4;
	}
	else if ((h[0] >= 'a') && (h[0] <= 'f'))
	{
		*b = (h[0] - 'a' + 10) << 4;
	}
	else
	{
		*b = 0;
		return;
	}
	if (h[1] == 0)
	{
		*b >>= 4;
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
void display_help(char *str)
{
	usart_tx("\r\n");
	if (str && str[0])
	{
		usart_tx("Invalid option: ");
		usart_tx(str);
		usart_tx("\r\n");
	}
	usart_tx("?: show help\r\n");
	usart_tx("h: show help\r\n");
	usart_tx("r: read flash\r\n");
	usart_tx("w <hex_val>: write flash\r\n");
}
void read_n_display_flash(uint8_t *start_addr, int len)
{
	int off;
	char ascii[3];
	uint8_t byte;

	for (off = 0; off < len; off++)
	{
		if (!(off & 0xF))
		{
			byte_to_hex((uint8_t)(off), ascii);
			usart_tx(ascii);
			usart_byte_tx(':');
		}
		byte = flash_read_byte((uint8_t *)(start_addr + off));
		usart_byte_tx(' ');
		byte_to_hex(byte, ascii);
		usart_tx(ascii);
		if (!((off + 1) & 0xF))
		{
			usart_tx("\r\n");
		}
	}
}
void write_flash(uint8_t *start_addr, int len, uint8_t val)
{
	int off, i;
	uint8_t buffer[BLOCK_SIZE];

	if (((uint16_t)start_addr & (BLOCK_SIZE - 1)) != 0) /* Not block aligned start address */
		return;
	if (((uint16_t)len & (BLOCK_SIZE - 1)) != 0) /* Not multiple of block size */
		return;

	for (off = 0; off < BLOCK_SIZE; off += 2)
	{
		buffer[off] = val;
		buffer[off + 1] = val ^ 0x1;
	}
	for (i = 0; i < len / BLOCK_SIZE; off++)
	{
		flash_write_block(start_addr + i * BLOCK_SIZE, buffer);
	}
}

int main(void)
{
	char str[16];
	int i;
	char value[3];
	uint8_t val;

	init_serial();

	usart_byte_rx(); // Waiting for a character, typically an <Enter>
	usart_tx("Welcome to eSrijan's Flash CLI\r\n");

	while (1)
	{
		usart_tx("eSrijan> ");
		usart_rx(str, 16);
		switch (str[0])
		{
			case 'h':
			case '?':
				if (str[1] != 0)
					display_help(str);
				else
					display_help(NULL);
				break;
			case 'r':
				if (str[1] != 0)
					display_help(str);
				else
					read_n_display_flash(FLASH_OPS_ADDR, FLASH_OPS_LEN);
				break;
			case 'w':
				if (str[1] != ' ')
					display_help(str);
				else
				{
					for (i = 2; str[i]; i++)
					{
						if (str[i] != ' ')
							break;
					}
					if (!isxdigit(str[i]))
						display_help(str);
					else
					{
						value[0] = str[i++];
						if (!str[i])
						{
							value[1] = 0;
						}
						else
						{
							if (!isxdigit(str[i]) || str[i + 1])
								display_help(str);
							else
							{
								value[1] = str[i];
								value[2] = 0;
							}
						}
						hex_to_byte(value, &val);
						write_flash(FLASH_OPS_ADDR, FLASH_OPS_LEN, val);
						read_n_display_flash(FLASH_OPS_ADDR, FLASH_OPS_LEN);
					}
				}
				break;
			default:
				display_help(str);
				break;
		}
	}

	return 0;
}
