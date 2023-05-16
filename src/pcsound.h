//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
// PC Speaker Interface
//

#ifndef __SDLSOUND_H__
#define __SDLSOUND_H__

void Speaker_Init(void);
void Speaker_Off(void);

//
// Play a particular tone to the speaker
//
// count: This determines the frequency. count is
//        the number of clock ticks between speaker
//        pulses. Hence clock_freq/count == tone
//        frequency
//

void Speaker_Output(unsigned short count);

extern int snd_tinnyfilter;

#endif
