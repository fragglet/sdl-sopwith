//
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
// System-independent video code
//

#include "video.h"

int keysdown[NUM_KEYS];

int Vid_GetGameKeys(void)
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
	if (keysdown[KEY_PULLUP]) {
		c |= K_FLAPU;
	}
	if (keysdown[KEY_PULLDOWN]) {
		c |= K_FLAPD;
	}
	if (keysdown[KEY_ACCEL]) {
		// smooth acceleration -- Jesse
		// keysdown[KEY_ACCEL] = 0;
		c |= K_ACCEL;
	}
	if (keysdown[KEY_DECEL]) {
		// smooth deacceleration -- Jesse
		// keysdown[KEY_DECEL] = 0;
		c |= K_DEACC;
	}
	if (keysdown[KEY_SOUND]) {
		keysdown[KEY_SOUND] = 0;
		c |= K_SOUND;
	}
	if (keysdown[KEY_BOMB]) {
		c |= K_BOMB;
	}
	if (keysdown[KEY_FIRE]) {
		c |= K_SHOT;
	}
	if (keysdown[KEY_HOME]) {
		c |= K_HOME;
	}
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
