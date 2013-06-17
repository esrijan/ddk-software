/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Header for I2C library
 */

#ifndef TWI_H
#define TWI_H

typedef enum
{
	standard,
	fast
} TwiMode;

void twi_init(TwiMode mode);
int twi_master_tx(uint8_t addr, uint8_t *data, int len);
int twi_master_rx(uint8_t addr, uint8_t *data, int len);
int twi_master_tx_rx(uint8_t addr, uint8_t *tx_data, int tx_len, uint8_t *rx_data, int rx_len);
#endif
