// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//
// System-independent video code
//
//---------------------------------------------------------------------------

#include "video.h"

int keysdown[NUM_KEYS];

int Vid_GetGameKeys()
{
	int i, c = 0;

	// empty input buffer, get new key state
	
	while (Vid_GetKey());
	
	if (Vid_GetCtrlBreak()) {
		c |= K_BREAK;
	}
	if (keysdown[KEY_FLIP]) {
		keysdown[KEY_FLIP] = 0;
		c |= K_FLIP;
	}
	if (keysdown[KEY_PULLUP])
		c |= K_FLAPU;
	if (keysdown[KEY_PULLDOWN])
		c |= K_FLAPD;
	if (keysdown[KEY_ACCEL]) {
		keysdown[KEY_ACCEL] = 0;
		c |= K_ACCEL;
	}
	if (keysdown[KEY_DECEL]) {
		keysdown[KEY_DECEL] = 0;
		c |= K_DEACC;
	}
	if (keysdown[KEY_SOUND]) {
		keysdown[KEY_SOUND] = 0;
		c |= K_SOUND;
	}
	if (keysdown[KEY_BOMB])
		c |= K_BOMB;
	if (keysdown[KEY_FIRE])
		c |= K_SHOT;
	if (keysdown[KEY_HOME])
		c |= K_HOME;
	if (keysdown[KEY_MISSILE]) {
		keysdown[KEY_MISSILE] = 0;
		c |= K_MISSILE;
	}
	if (keysdown[KEY_STARBURST]) {
		keysdown[KEY_STARBURST] = 0;
		c |= K_STARBURST;
	}

	// clear bits in key array
	
	for (i=0; i<NUM_KEYS; ++i) {
		keysdown[i] &= ~2;
	}

	return c;
}

//-----------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/03/26 13:53:29  fraggle
// Allow control via arrow keys
// Some code restructuring, system-independent video.c added
//
//
//-----------------------------------------------------------------------
