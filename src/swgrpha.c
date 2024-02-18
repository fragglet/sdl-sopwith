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
//        swgrph   -      SW screen graphics
//

#include "video.h"

#include "sw.h"
#include "swend.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swmove.h"
#include "swsplat.h"
#include "swstbar.h"
#include "swtext.h"

static void dispgrnd(void)
{
	if (conf_solidground) {
		Vid_DispGround_Solid(ground + displx);
	} else {
		Vid_DispGround(ground + displx);
	}
}

void swground(void)
{
	dispgrnd();
}


// sdh 14/2/2003: find the color of an object
// always draw bullets white

static inline int ob_color(OBJECTS *ob)
{
	if (ob->ob_type == SHOT) {
		return 3;
	} else {
		return ob->ob_clr;
	}
}

void swputsym(int x, int y, OBJECTS * ob)
{
	Vid_DispSymbol(x, y, ob->ob_newsym, ob_color(ob));
}

static void print_help(void)
{
	char buf[64];
	int i;
	struct {
		char *name; int key;
	} items[] = {
		{ "Accelerate",  KEY_ACCEL },
		{ "Decelerate",  KEY_DECEL },
		{ "Pull Up",     KEY_PULLUP },
		{ "Pull Down",   KEY_PULLDOWN },
		{ "Flip Plane",  KEY_FLIP },
		{ "Fire Gun",    KEY_FIRE },
		{ "Drop Bomb",   KEY_BOMB },
		{ "Fly Home",    KEY_HOME },
	};

	// We usually only show the help text in novice mode. However, in
	// other single player modes, we do show the help text if the
	// player seems to be really struggling.
	switch (playmode) {
		case PLAYMODE_NOVICE:
			break;
		case PLAYMODE_SINGLE:
		case PLAYMODE_COMPUTER:
			if (consoleplayer->ob_crashcnt < 3) {
				return;
			}
			break;
		default:
			return;
	}

	swcolor(2);
	swposcur(1, 2);
	swputs("BEGINNER'S HELP");
	swcolor(3);
	for (i = 0; i < arrlen(items); i++) {
		snprintf(buf, sizeof(buf), "%-11s- %s",
		        items[i].name, Vid_KeyName(keybindings[items[i].key]));
		swposcur(1, i + 3);
		swputs(buf);
	}
	snprintf(buf, sizeof(buf), "%-11s- %s", "Restart", "Ctrl-R");
	swposcur(1, i + 3);
	swputs(buf);
	++i;

	snprintf(buf, sizeof(buf), "%-11s- %s", "End Game", "Ctrl-Q");
	swposcur(1, i + 3);
	swputs(buf);
}

void swdisp(void)
{
	OBJECTS *ob;

	Vid_ClearBuf();

	// display the status bar
	dispstatusbar();

	// heads up splats
	if (conf_hudsplats) {
		swdispsplats();
	}

	// "the end"
	dispendmessage();

	// Display help text if the player is just starting off. We only
	// show this on the first level, and stop showing it once the
	// player demonstrates the ability to take off successfully.
	if (consoleplayer->ob_athome && !successful_flight && gamenum == 0
	 && consoleplayer->ob_state == FLYING) {
		print_help();
	}

	// calculate displx from the player position
	// do sanity checks to make sure we never go out of range
	displx = consoleplayer->ob_x - SCR_CENTR;

	if (displx < 0) {
		displx = 0;
	} else if (displx >= currgame->gm_max_x - SCR_WDTH) {
		displx = currgame->gm_max_x - SCR_WDTH - 1;
	}

	// draw objects
	for (ob = objtop; ob; ob = ob->ob_next) {
		int x, y;

		x = ob->ob_x;
		y = ob->ob_y;

		if (ob->ob_drwflg && x >= displx && x < displx + SCR_WDTH) {
			swputsym(x - displx, y, ob);

			if (ob->ob_drawf) {
				(*(ob->ob_drawf)) (ob);
			}
		}
	}

	dispgrnd();

	// need to update the screen as we arent writing
	// directly into vram any more
	Vid_Update();
}


void colorscreen(int color)
{
	int x, y;

	for (y=19; y<SCR_HGHT; ++y) {
		for (x=0; x<SCR_WDTH; ++x) {
			Vid_PlotPixel(x, y, color);
		}
	}
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// 		  fix bullets being colored (should be white)
// sdh 27/7/2002: removed old collision detection code
// sdh 27/6/2002: move to new sopsym_t for symbols
// sdh 26/03/2002: moved all drawing functions into platform specific
//                 files
//                 change CGA_ to Vid_
// sdh 28/10/2001: get_type/set_type removed
// sdh 28/10/2001: moved auxdisp and auxdisp functions here
// sdh 24/10/2001: fix auxdisp buffer
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: added #define for solid ground (sopwith 1 style)
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed extern definitions, these are in headers now
//                 shuffled some functions round to shut up the compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-03-09        Microsoft compiler.
// 85-11-05        Atari
// 84-06-13        PCjr Speed-up
// 84-02-21        Development
//
