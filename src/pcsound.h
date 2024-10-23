//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// You can redistribute and/or modify this program under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation, or any later version. This program is distributed WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
//
//
// PC Speaker Interface
//

#ifndef __SDLSOUND_H__
#define __SDLSOUND_H__

#include "std.h"

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

extern bool snd_tinnyfilter;

#endif
