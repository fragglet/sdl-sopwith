// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2001-2003 Simon Howard
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
// $Log$
// Revision 1.1  2003/02/14 19:03:37  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: added cvs tags
//
//--------------------------------------------------------------------------
