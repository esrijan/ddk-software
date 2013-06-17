/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Header for DDK specific definitions
 */
#ifndef DDK_H
#define DDK_H

#if (DDK_VER == 2)
// Pull-up on PB2 for switch
#define BUTTON_INIT PORTB |= 0b00000100
#define BUTTON_PRESSED (!(PINB & (1 << PB2)))
#define BUTTON_RELEASED (!!(PINB & (1 << PB2)))
#else
#define BUTTON_INIT
#define BUTTON_PRESSED (!!(PINB & (1 << PB2)))
#define BUTTON_RELEASED (!(PINB & (1 << PB2)))
#endif

#endif
