/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 *
 * Header for Character LCD Functions
 */

#ifndef CLCD_H
#define CLCD_H

#include <avr/io.h>

#define CLCD_AUTO_POS

void clcd_init(void);
void clcd_cls(void);
void clcd_home(void);
void clcd_move_to(uint8_t pos); /* 0-31 */
void clcd_scroll_left(int num_chars, int delay);
void clcd_scroll_right(int num_chars, int delay);
void clcd_data_wr(uint8_t data);
void clcd_cmd_wr(uint8_t cmd);
void clcd_print_string(char *str);
#endif
