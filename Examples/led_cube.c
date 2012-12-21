/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Control the LED Cube. Hardware connection details as follows:
 * + 2 cascaded 8-bit shift register control: PA0 - Data, PA1 - CLK, PA7 - OE
 * + First shift register controls
 * 	> Q1 - (0, 0); Q2 - (1, 0); Q3 - (2, 0); Q4 - (3, 0)
 * 	> Q5 - (0, 1); Q6 - (1, 1); Q7 - (2, 1); Q8 - (3, 1)
 * + Second shift register controls
 * 	> Q1 - (0, 2); Q2 - (1, 2); Q3 - (2, 2); Q4 - (3, 2)
 * 	> Q5 - (0, 3); Q6 - (1, 3); Q7 - (2, 3); Q8 - (3, 3)
 * + Z controls: PC0 - 0; PC1 - 1; PC6 - 2; PC7 - 3
 */

#define INT_BASED

#include <avr/io.h>
#include <stdlib.h>
#ifdef INT_BASED
#include <avr/interrupt.h>
#endif
#include <util/delay.h>

// Define LED xy-layer control port
// 4x4 grid is controlled through 2 cascaded 8-bit shift registers,
// mapped using DATA, CLK, OE over PORTA
#define PORT_XY PORTA
#define DDR_XY DDRA
#define DATA PA0
#define CLK PA1
#define OE PA7
// Define LED z-layer control port
#define PORT_Z PORTC
#define DDR_Z DDRC

uint8_t cube_zy[4][4];
uint8_t z_mask[4] = { 0x01, 0x02, 0x40, 0x80 }, z_cur = 0;

void init_io(void)
{
	// 1 = output, 0 = input
	DDR_XY = 0b11111111; // All outputs
	DDR_Z = 0b11111111; // All outputs

#ifdef INT_BASED
	// Enable interrupts to start drawing the cube buffer.
	// When interrupts are enabled, ISR(TIMER2_COMP_vect)
	// will run on timed intervalls.
	sei();
	TIMSK |= (1 << OCIE2); // Enable CTC interrupt

	// Initiate timer - Derived from reference code

	// Frame buffer interrupt
	TCNT2 = 0x00;	// initial counter value = 0;
	// Every 1024th cpu cycle, a counter is incremented.
	// Every time that counter reaches 15, it is reset to 0,
	// and the interrupt routine is executed.
	// 16000000/1024/16 = ~976 times per second
	// There are 4 layers to update..
	// 16000000/1024/16/4 = 244 FPS
	// == flicker free :)
	OCR2 = 16; 			// interrupt at counter = 16
	TCCR2 = 0x05; 		// prescaler = 1024
	TCCR2 |= (1 << WGM01);	// Clear Timer on Compare Match (CTC) mode
#endif
}

void set_led(void)
{
	PORT_XY |= (1 << DATA);
	PORT_XY |= (1 << CLK);
	_delay_us(1);
	PORT_XY &= ~(1 << CLK);
	_delay_us(1);
}
void reset_led(void)
{
	PORT_XY &= ~(1 << DATA);
	PORT_XY |= (1 << CLK);
	_delay_us(1);
	PORT_XY &= ~(1 << CLK);
	_delay_us(1);
}

#ifdef INT_BASED
// Cube buffer draw interrupt routine
ISR(TIMER2_COMP_vect)
{
	int y, x;

	// We don't want to see the cube updating.
	PORT_XY &= ~(1 << OE);
	// Display the current 2D image at the current layer along the Z axis
	for (y = 3; y >= 0; y--)
	{
		for (x = 3; x >= 0; x--)
		{
			(cube_zy[z_cur][y] & (1 << x)) ? set_led() : reset_led();
		}
	}
	// Enable the apropriate layer
	PORT_Z = z_mask[z_cur];
	PORT_XY |= (1 << OE);

	// The cube only has 4 layers (0,1,2,3)
	// If we are at layer 3 now, we want to go back to layer 0.
	if (z_cur++ == 3)
		z_cur = 0;
}
void display_cube(int delay)
{
	_delay_ms(delay);
}
#else
void display_cube(int delay)
{
	int z, y, x;

	do
	{
		for (z = 3; z >= 0; z--)
		{
			PORT_XY &= ~(1 << OE);
			for (y = 3; y >= 0; y--)
			{
				for (x = 3; x >= 0; x--)
				{
					(cube_zy[z][y] & (1 << x)) ? set_led() : reset_led();
				}
			}
			PORT_Z = z_mask[z];
			PORT_XY |= (1 << OE);
			_delay_ms(1);
			delay--;
		}
	}
	while (delay > 0);
}
#endif

void clear_cube(void)
{
	uint8_t z, y;

	for (z = 0; z < 4; z++)
	{
		for (y = 0; y < 4; y++)
		{
			cube_zy[z][y] = 0;
		}
	}
}
void set_coord(uint8_t x, uint8_t y, uint8_t z)
{
	cube_zy[z][y] |= (1 << x);
}
void reset_coord(uint8_t x, uint8_t y, uint8_t z)
{
	cube_zy[z][y] &= ~(1 << x);
}

void on_off(int speed)
{
	int x, y, z;

	for (z = 0; z < 4; z++)
		for (y = 0; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				set_coord(x, y, z);
				display_cube(4 * speed);
			}
	for (z = 0; z < 4; z++)
		for (y = 0; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				reset_coord(x, y, z);
				display_cube(4 * speed);
			}
}
void rand_pixel(void)
{
	int cnt;
	int x, y, z;

	for (cnt = 0; cnt < 100; cnt++)
	{
		x = rand() % 4;
		y = rand() % 4;
		z = rand() % 4;
		set_coord(x, y, z);
		display_cube(100);
	}
	clear_cube();
	display_cube(100);
}
static void circle(int x, int big, int on, int delay)
{
	void (*coord)(uint8_t, uint8_t, uint8_t) = (on ? set_coord : reset_coord);

	if (big)
	{
		coord(x, 1, 0);
		coord(x, 2, 0);
		coord(x, 0, 1);
		coord(x, 3, 1);
		coord(x, 0, 2);
		coord(x, 3, 2);
		coord(x, 1, 3);
		coord(x, 2, 3);
	}
	else
	{
		coord(x, 1, 1);
		coord(x, 2, 1);
		coord(x, 1, 2);
		coord(x, 2, 2);
	}
	display_cube(delay);
}
void rings(void)
{
	int i = 0, delay = 50;

	while (i < 5)
	{
		circle(0, 1, 1, delay);
		circle(0, 0, 0, delay);
		circle(1, 0, 1, delay);
		circle(0, 1, 0, delay);
		circle(2, 1, 1, delay);
		circle(1, 0, 0, delay);
		circle(3, 0, 1, delay);
		circle(2, 1, 0, delay);
		circle(3, 1, 1, delay);
		circle(3, 0, 0, delay);
		circle(2, 0, 1, delay);
		circle(3, 1, 0, delay);
		circle(1, 1, 1, delay);
		circle(2, 0, 0, delay);
		circle(0, 0, 1, delay);
		circle(1, 1, 0, delay);
		i++;
	}
	circle(0, 0, 0, delay);
}
static void xz_plane(int y, int on, int delay)
{
	void (*coord)(uint8_t, uint8_t, uint8_t) = (on ? set_coord : reset_coord);
	int x, z;

	for (x = 0; x < 4; x++)
		for (z = 0; z < 4; z++)
			coord(x, y, z);
	display_cube(delay);
}
void waves(void)
{
	int i = 0, delay = 125;

	while (i < 10)
	{
		xz_plane(0, 1, delay);
		xz_plane(3, 0, delay);
		xz_plane(1, 1, delay);
		xz_plane(0, 0, delay);
		xz_plane(2, 1, delay);
		xz_plane(1, 0, delay);
		xz_plane(3, 1, delay);
		xz_plane(2, 0, delay);
		i++;
	}
	xz_plane(3, 0, delay);
}
#ifdef REFRESH_CTL_AT_USEC // TODO
void intensity(void)
{
	int s;
	int x, y, z;

	for (s = 0; s < 20; s++)
	{
		for (z = 0; z < 4; z++)
			for (y = 0; y < 4; y++)
				for (x = 0; x < 4; x++)
				{
					set_coord(x, y, z);
				}
		_delay_us(200 * s); // TODO: Not this but the refreshing would control intensity
		for (z = 0; z < 4; z++)
			for (y = 0; y < 4; y++)
				for (x = 0; x < 4; x++)
				{
					reset_coord(x, y, z);
				}
		_delay_us(2000); // TODO: Not this but the refreshing would control intensity
	}
}
#endif

int main(void)
{
	int s;

	init_io();

	clear_cube();
	while (1)
	{
		for (s = 1; s < 5; s++)
			on_off(s);
		_delay_ms(1000);
		rand_pixel();
		_delay_ms(1000);
		rings();
		_delay_ms(1000);
		waves();
		_delay_ms(1000);
#ifdef REFRESH_CTL_AT_USEC
		intensity();
		_delay_ms(1000);
#endif
	}

	return 0;
}
