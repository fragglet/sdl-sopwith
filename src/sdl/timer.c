// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 2001 Simon Howard
//
// This file is dual-licensed under version 2 of the GNU General Public
// License as published by the Free Software Foundation, and the Sopwith
// License as published by David L. Clark. See the files GPL and license.txt
// respectively for more information.
//
//--------------------------------------------------------------------------
//
// Timer code
//
// Abstraction layer for access to the timer
// Basically this is to keep SDL out of the main code, so we can if we
// want drop in other code to run without SDL.
//
//--------------------------------------------------------------------------

#include <SDL.h>
#include "sw.h"

// get time in milliseconds

int Timer_GetMS()
{
	return SDL_GetTicks();
}

void Timer_Sleep(int usec)
{
	SDL_Delay(usec);
}

void Timer_Init()
{
	SDL_Init(SDL_INIT_TIMER);
}

//--------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
//
//--------------------------------------------------------------------------
