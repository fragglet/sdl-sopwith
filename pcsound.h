// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// PC Speaker routines
//
//---------------------------------------------------------------------------

#ifndef __SDLSOUND_H__
#define __SDLSOUND_H__

//
// Initialise and Shut down
//

void Speaker_Init();
void Speaker_Shutdown();

//
// Turn speaker on and off
//

void Speaker_On();
void Speaker_Off();

// 
// Play a particular tone to the speaker
//
// count: This determines the frequency. count is 
//        the number of clock ticks between speaker
//        pulses. Hence clock_freq/count == tone 
//        frequency
//

void Speaker_Output(unsigned short count);

//
// Play a sound 
//
// This should be identical to the BASIC SOUND function
// duration = 1/18th's of a second
//

void Speaker_Sound(int freq, int duration);

#endif

