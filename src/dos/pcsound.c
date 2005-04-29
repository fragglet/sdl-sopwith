// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------
//
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//
//-----------------------------------------------------------------------
//
// PC Speaker Code
//
//-----------------------------------------------------------------------

// sopwith note: this is a small library i wrote a while back to
// drive the pc speaker code. it is api compatible with the pc
// speaker "emulation" code i wrote for sopwith, so will probably
// be of use

#include <time.h>
#include <pc.h>

//#define DEBUG

#define CLOCK_FREQ   1193280
#define PPI_PORT     97
#define TIMER_PORT_1 66
#define TIMER_PORT_2 67

static int speaker_on = 0;

//
// Turn the speaker off
//

void Speaker_Off()
{
	// if already off, dont turn off

	if(!speaker_on)
		return;

#ifdef DEBUG
	puts("turn speaker off");
#endif

	// turn off speaker

	outportb(PPI_PORT, inportb(PPI_PORT) & ~3);

	speaker_on = 0;
}

//
// Turn the speaker on
//
// (this is kinda internal)
//

void Speaker_On()
{
	if(speaker_on)
		return;

#ifdef DEBUG
	puts("turn speaker on");
#endif

	// turn on speaker

	outportb(PPI_PORT, inportb(PPI_PORT) | 3);

	speaker_on = 1;
}

static unsigned int speaker_initted = 0;

//
// Shutdown the speaker
//

void Speaker_Shutdown()
{
	if(!speaker_initted)
		return;

#ifdef DEBUG
	puts("shutdown");
#endif

	// turn the speaker off before we finish

	Speaker_Off();

	speaker_initted = 0;
}

//
// Initialise the speaker
//

void Speaker_Init()
{
	if(speaker_initted)
		return;

	speaker_on = 0;
	speaker_initted = 1;

	{
		static int firsttime = 1;
		
		if(firsttime) {
			atexit(Speaker_Shutdown);
			firsttime = 0;
		}
	}
}

// 
// Play a particular tone to the speaker
//
// count: This determines the frequency. count is 
//        the number of clock ticks between speaker
//        pulses. Hence clock_freq/count == tone 
//        frequency
//

void Speaker_Output(unsigned short count)
{
	// init if we havent already

	Speaker_Init();
	Speaker_On();

	// get timer ready
	// 182 is the code to indicate a new count value

	outportb(TIMER_PORT_2, 182);

	// send value to other port

	outportb(TIMER_PORT_1, count & 255);
	outportb(TIMER_PORT_1, (count >> 8) & 255);
}

//
// Play a sound 
//
// This should be identical to the BASIC SOUND function
// duration = 1/18th's of a second
//

void Speaker_Sound(int freq, int duration)
{
	clock_t duration_clocks = duration * CLOCKS_PER_SEC / 18.2;
	clock_t endtime;

	// turn speaker on

	int count = CLOCK_FREQ / freq;

	Speaker_Output(count);

	// delay

	for(endtime = clock() + duration_clocks; clock() < endtime; );

	// turn speaker off

	Speaker_Off();
}

//-----------------------------------------------------------------------
//
// $Log$
// Revision 1.2  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.1.1.1  2003/02/14 19:03:34  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
//
//-----------------------------------------------------------------------

