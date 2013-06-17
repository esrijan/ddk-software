/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Play our national anthem (jana gana mana)
 * Buzzer/Speaker is assumed to be connected between OC2/PD7 (Pin 21) & GND.
 * Musical note frequency generation is done using CTC of 8-bit Timer/Counter2.
 * Replay of the anthem can be done by pressing the switch connected to PB2.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"
#include "notes.h"

/*
 * The following has been decided based on the possible_freq.txt and by
 * actually hearing the audio
 */
#define PRESCALER (6 << CS20)
#define MAX_FREQ ((F_CPU / 256) / 2)

/* Volume will be always 50% as duty cycle would be always 50% */
//int vol = 50;

#if 1
/* Preset notes for Indian National Anthem */
static const Note notes[] = {
    /* Jana Gana Mana Adhinayaka Jaya He */
    {_s, 0}, {_r, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_g, 0}, {_m, 0}, {_NOTE_DELAY},
    /* Bharatha Bhagya Vidhatha */
    {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_r, 0}, {_r, 0}, {_r, 0}, {__n, 0}, {_r, 0}, {_s, 0}, {_s, 0}, {_NOTE_DELAY},
    /* Punjaba Sindhu Guja    ratha Mara     tha */
    {_s, 0}, {_s, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_m, 0}, {_d, 0}, {_p, 0}, {_NOTE_DELAY},
    /* Dravida Utkala Vangha */
    {_m, 0}, {_m, 0}, {_m, 0}, {_m, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_m, 0}, {_g, 0}, {_NOTE_DELAY},
    /* Vindhya Himaachala Yamuna Ganga */
    {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_p, 0}, {_p, 0}, {_m, 0}, {_m, 0}, {_m, 0}, {_m, 0}, {_m, 0}, {_NOTE_DELAY},
    /* Uchchala Jaladhi Taranga */
    {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_r, 0}, {_r, 0}, {_r, 0}, {__n, 0}, {_r, 0}, {_s, 0}, {_NOTE_DELAY},
    /* Tava Shubha Name Jage */
    {_s, 0}, {_r, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_g, 0}, {_m, 0}, {_NOTE_DELAY},
    /* Tava Shubha Aashisha Maage */
    {_g, 0}, {_m, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_m, 0}, {_g, 0}, {_r, 0}, {_m, 0}, {_g, 0}, {_NOTE_DELAY},
    /* Gahe Tava Jaya Gaatha */
    {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_r, 0}, {_r, 0}, {_r, 0}, {__n, 0}, {_r, 0}, {_s, 0}, {_NOTE_DELAY},
    /* Jana Gana Mangala   Daayaka Ja   ya he */
    {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_p, 0}, {_m, 0}, {_d, 0}, {_p, 0}, {_NOTE_DELAY},
    /* Bharatha Bhagya Vi  datha */
    {_m, 0}, {_m, 0}, {_m, 0}, {_m, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_m, 0}, {_g, 0}, {_NOTE_DELAY},
    /* Jaya He Jaya He Jaya He */
    {_S, 0}, {_n, 0}, {_S, 0}, {_n, 0}, {_S, 0}, {_n, 0}, {_d, 0}, {_n, 0}, {_d, 0}, {_NOTE_DELAY},
    //{_n, 0}, {_n, 0}, {_S, 0}, {_S, 0}, {_S, 0}, {_S, 0}, {_S, 0}, {_S, 0}, {_d, 0}, {_d, 0}, {_n, 0}, {_n, 0}, {_n, 0}, {_n, 0}, {_n, 0}, {_n, 0}, {_p, 0}, {_p, 0}, {_d, 0}, {_d, 0}, {_d, 0}, {_d, 0}, {_d, 0}, {_d, 0}, {_NOTE_DELAY},
    /* Jaya Jaya Jaya Jaya He */
    {_s, 0}, {_s, 0}, {_r, 0}, {_r, 0}, {_g, 0}, {_g, 0}, {_r, 0}, {_g, 0}, {_m, 0}, {_NOTE_DELAY},
    /** Stop **/
    {0, 0}
};
#else
static const Note notes[] = {{_C4, 0},{_D4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_D4, 0},{_E4, 0},{_F4, 0},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_D4, 0},{_ZZ},{_D4, 0},{_D4, 0},{_B3, 0},{_D4, 0},{_C4, 0},{_ZZ},{_C4, 0},{_ZZ},{_C4, 0},{_ZZ},{_ZZ},{_C4, 0},{_ZZ},{_G4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_G4, 0},{_G4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_F4, 0},{_A4, 0},{_G4, 0},{_ZZ},{_F4, 0},{_ZZ},{_F4, 0},{_F4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_D4, 0},{_F4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_D4, 0},{_G4, 0},{_G4, 0},{_F4, 0},{_ZZ},{_F4, 0},{_ZZ},{_F4, 0},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_D4, 0},{_ZZ},{_D4, 0},{_D4, 0},{_B3, 0},{_D4, 0},{_C4, 0},{_ZZ},{_C4, 0},{_ZZ},{_C4, 0},{_ZZ},{_ZZ},{_C4, 0},{_D4, 0},{_E4, 0},{_E4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_D4, 0},{_E4, 0},{_F4, 0},{_ZZ},{_F4, 0},{_ZZ},{_F4, 0},{_ZZ},{_E4, 0},{_F4, 0},{_G4, 0},{_G4, 0},{_G4, 0},{_ZZ},{_F4, 0},{_E4, 0},{_D4, 0},{_F4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_D4, 0},{_ZZ},{_D4, 0},{_D4, 0},{_B3, 0},{_D4, 0},{_C4, 0},{_ZZ},{_C4, 0},{_ZZ},{_C4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_G4, 0},{_G4, 0},{_G4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_G4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_F4, 0},{_A4, 0},{_G4, 0},{_F4, 0},{_ZZ},{_F4, 0},{_F4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_E4, 0},{_D4, 0},{_F4, 0},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_E4, 0},{_ZZ},{_B4, 0},{_B4, 0},{_C5, 0},{_ZZ},{_C5, 0},{_ZZ},{_C5, 0},{_ZZ},{_A4, 0},{_A4, 0},{_B4, 0},{_ZZ},{_B4, 0},{_ZZ},{_B4, 0},{_ZZ},{_G4, 0},{_G4, 0},{_A4, 0},{_ZZ},{_A4, 0},{_ZZ},{_A4, 0},{_ZZ},{_ZZ},{_C4, 0},{_C4, 0},{_D4, 0},{_D4, 0},{_E4, 0},{_E4, 0},{_D4, 0},{_E4, 0},{_F4, 0},{_ZZ},{0, 0}};
#endif

void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	DDRB &= ~(1 << PB2); // Set PB2 as input
}
void set_frequency(unsigned long freq)
/* f = MAX_FREQ / (1 + OCR2), i.e. OCR2 = MAX_FREQ / f - 1; */
{
	if (freq < (MAX_FREQ / 256))
	{
		freq = (MAX_FREQ / 256);
	}
	else if (freq > MAX_FREQ)
	{
		freq = MAX_FREQ;
	}
	OCR2 = MAX_FREQ / freq - 1;
}
void init_timer(void)
{
	DDRD |= (1 << PD7); // OC2 is PD7

	set_frequency(_s); // sa to start with

	/*
	 * Setting the Timer/Counter2 in CTC (Clear Timer on Compare) (non-PWM) for
	 * controlling frequency of waveforms, directly by the compare register &
	 * Toggling on Match to generate square wave for a particular frequency.
	 * Output would come on OC2/PD7 (Pin 21).
	 */
	TCCR2 = (1 << WGM21) | (0 << WGM20) | (1 << COM20) | PRESCALER;
}
void stop_timer(void)
{
	TCCR2 = 0;
}
void play_note(const Note *n)
{
    if (n->freq != _NOTE_DELAY_FREQ)
	{
        set_frequency(n->freq);
    }
    if (n->delay_in_ms)
	{
        _delay_ms(notes->delay_in_ms);
    }
    else
	{
        _delay_ms(OCTAVE_DELAY_IN_MS);
    }
}
void play_music(void)
// Play preset notes
{
    int i;

    for (i = 0; notes[i].freq; i++)
	{
        play_note(&notes[i]);
    }
}
void play_anthem(void)
{
	init_timer();
	play_music();
	stop_timer();
}

int main(void)
{
	init_io();
	play_anthem();

	while (1)
	{
		if (BUTTON_PRESSED)
		{
			play_anthem();
		}
	}

	return 0;
}
