/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * Generic
 * 
 * Header for musical note frequencies used in national anthem (jana gana mana)
 */
#ifndef NOTES_H
#define NOTES_H

#define OCTAVE_FREQ_BASE (440) // A4
#define OCTAVE_FREQ_DIFF (1.059463) // 2^(1/12)
#define OCTAVE_NEXT(c) (int)((c) * OCTAVE_FREQ_DIFF + 0.5)

#define OCTAVE_DELAY_IN_MS (391) // in ms

#define _NOTE_DELAY_FREQ (-1)
#define _NOTE_DELAY -1, (OCTAVE_DELAY_IN_MS / 2)
#define _ZZ -1, (OCTAVE_DELAY_IN_MS / 2)

#define _A4 OCTAVE_FREQ_BASE
#define _A4S OCTAVE_NEXT(_A4)
#define _B4 OCTAVE_NEXT(_A4S)
#define _C4 OCTAVE_NEXT(_B4)
#define _C4S OCTAVE_NEXT(_C4)
#define _D4 OCTAVE_NEXT(_C4S)
#define _D4S OCTAVE_NEXT(_D4)
#define _E4 OCTAVE_NEXT(_D4S)
#define _F4 OCTAVE_NEXT(_E4)
#define _F4S OCTAVE_NEXT(_F4)
#define _G4 OCTAVE_NEXT(_F4S)
#define _G4S OCTAVE_NEXT(_G4)

// Specifically for National Anthem
#define _B3 247
#define _C5 523

#define _s _C4
#define _r1 _C4S
#define _r2 _D4
#define _g1 _D4
#define _r3 _D4S
#define _g2 _D4S
#define _g3 _E4
#define _m1 _F4
#define _m2 _F4S
#define _p _G4
#define _d1 _G4S
#define _d2 _A4
#define _n1 _A4
#define _d3 _A4S
#define _n2 _A4S
#define _n3 _B4

// Specifically for National Anthem
#define __n3 _B3
#define _S _C5

//#define _s _s
#define _r _r2
#define _g _g3
#define _m _m1
//#define _p _p
#define _d _d2
#define _n _n3

// Specifically for National Anthem
#define __n __n3
//#define _S _S

typedef struct
{
	int freq;
	int delay_in_ms;
} Note;

#endif
