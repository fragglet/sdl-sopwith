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
//        swdispc  -      Display all players and objects
//

#include "sw.h"
#include "swdisp.h"
#include "swsound.h"

static void plnsound(OBJECTS *ob)
{
	if (ob->ob_firing) {
		sound(S_SHOT, 0, ob);
	} else {
		switch (ob->ob_state) {
		case FALLING:
			if (ob->ob_dy >= 0) {
				sound(S_HIT, 0, ob);
			} else {
				sound(S_FALLING, ob->ob_y, ob);
			}
			break;

		case FLYING:
			sound(S_PLANE, -ob->ob_speed, ob);
			break;

		case STALLED:
		case WOUNDED:
		case WOUNDSTALL:
			sound(S_HIT, 0, ob);
			break;

		default:
			break;
		}
	}
}

void dispbomb(OBJECTS *ob)
{
	if (ob->ob_dy <= 0) {
		sound(S_BOMB, -ob->ob_y, ob);
	}
}

void dispexpl(OBJECTS *ob)
{
	if (ob->ob_orient) {
		sound(S_EXPLOSION, ob->ob_hitcount, ob);
	}
}

void dispcomp(OBJECTS * ob)
{
	plnsound(ob);
}

void disptarg(OBJECTS * ob)
{
	if (ob->ob_firing) {
		sound(S_SHOT, 0, ob);
	}
}

void dispplyr(OBJECTS * ob)
{
	plnsound(ob);
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/06/2002: move to new sopsym_t for symbols,
//                 remove symwdt and symhgt
// sdh 28/10/2001: option to disable hud splats
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed extern definitions: these are now in headers
//                 shuffled some functions around to shut up compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-04-05        Missile and starburst support
// 87-03-13        Splatted bird symbol.
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 84-06-12        PCjr Speed-up
// 84-02-21        Development
//

