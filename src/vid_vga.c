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
//  Graphics backend for 8-bit (VGA) displays
//

#include <string.h>
#include <assert.h>

#include "video.h"
#include "sw.h"
#include "swsymbol.h"

// this should be in a header somewhere
#define SBAR_HGHT 19

#define VRAMSIZE (SCR_HGHT * vid_pitch)

unsigned char *vid_vram;
unsigned int vid_pitch;

// Draw SCR_WDTH columns of ground, starting at the given ground pointer.
void Vid_DispGround(GRNDTYPE *gptr)
{
	GRNDTYPE *g = gptr;
	unsigned char *sptr;
	int x, y;
	int gc, gl;

	gl = *g;
	if (gl >= SCR_HGHT - 1) {
		gl = SCR_HGHT - 1;
	}

	sptr = vid_vram + (SCR_HGHT-1 - gl) * vid_pitch;

	for (x=SCR_WDTH, g = gptr; x>0; --x) {
		gc = *g++;
		if (gc >= SCR_HGHT - 1) {
			gc = SCR_HGHT - 1;
		}
		if (gl == gc) {
			*sptr ^= 3;
		} else if (gc < gl) {
			for (y = gl - gc; y; --y) {
				*sptr ^= 3;
				sptr += vid_pitch;
			}
		} else {
			for (y = gc - gl; y; --y) {
				*sptr ^= 3;
				sptr -= vid_pitch;
			}
		}
		gl = gc;
		++sptr;
	}
}


// sdh 28/10/2001: solid ground function
void Vid_DispGround_Solid(GRNDTYPE * gptr)
{
	GRNDTYPE *g = gptr;
	unsigned char *sptr;
	int x, y;
	int gc;

	for (x=0, g = gptr; x<SCR_WDTH; ++x) {
		gc = *g++;
		if (gc >= SCR_HGHT - 1) {
			gc = SCR_HGHT - 1;
		}

		sptr = vid_vram + (SCR_HGHT-SBAR_HGHT-1) * vid_pitch + x;
	
		for (y = gc-SBAR_HGHT+1; y; --y) {
			*sptr ^= 3;
			sptr -= vid_pitch;
		}
	}
}

void Vid_PlotPixel(int x, int y, int clr)
{
	unsigned char *sptr
		= vid_vram + (SCR_HGHT-1 - y) * vid_pitch + x;

	*sptr = clr & 3;
}

void Vid_XorPixel(int x, int y, int clr)
{
	unsigned char *sptr
		= vid_vram + (SCR_HGHT-1 - y) * vid_pitch + x;

	*sptr ^= clr & 3;
}

static unsigned char color_mappings[][4] = {
	{ 0, 3, 3, 3 },  // All-white                     - OWNER_NONE?
	{ 0, 1, 2, 3 },  // Cyan fuselage, magenta wings  - OWNER_PLAYER1
	{ 0, 2, 1, 3 },  // Magenta fuselage, cyan wings  - OWNER_PLAYER2
	// New colors:
	{ 0, 1, 3, 2 },  // Cyan fuselage, white wings    - OWNER_PLAYER3
	{ 0, 2, 3, 1 },  // Magenta fuselage, white wings - OWNER_PLAYER4
	{ 0, 3, 1, 2 },  // White fuselage, cyan wings    - OWNER_PLAYER5
	{ 0, 3, 2, 1 },  // White fuselage, magenta wings - OWNER_PLAYER6
	// Now we're getting into boring territory...
	{ 0, 1, 1, 3 },  // All-cyan                      - OWNER_PLAYER7
	{ 0, 2, 2, 3 },  // All-magenta                   - OWNER_PLAYER8
};

// Given a player number (ob_owner_t), returns the color of the fuselage
// when that plane is drawn to the screen. This is used by the map to get
// the color of objects when they're drawn there.
// TODO: When the fuselage is white, we should probably return the wing
// color instead so that the player can be distinguished from the ground.
int Vid_FuselageColor(ob_owner_t clr)
{
	assert(clr < arrlen(color_mappings));
	return color_mappings[clr][1];
}

void Vid_DispSymbol(int x, int y, sopsym_t *symbol, ob_owner_t clr)
{
	unsigned char *sptr = vid_vram + (SCR_HGHT-1 - y) * vid_pitch + x;
	unsigned char *data = symbol->data;
	int x1, y1;
	int w = symbol->w, h = symbol->h;
	int wrap = x - SCR_WDTH + w;
	unsigned char *color_mapping;

	if (w == 1 && h == 1) {
		Vid_XorPixel(x, y, clr);
		return;
	}

	if (wrap > 0) {
		//wrap += 4;
		w -= wrap;
	} else {
		wrap = 0;
	}

	if (h > y + 1) {
		h = y + 1;
	}

	assert(clr < arrlen(color_mappings));
	color_mapping = color_mappings[clr];
	for (y1=0; y1<h; ++y1) {
		unsigned char *sptr2 = sptr;
		for (x1=0; x1<w; ++x1, ++sptr2) {
			int i = *data++;

			if (i) {
				*sptr2 ^= color_mapping[i];
			}
		}
		data += wrap;
		sptr += vid_pitch;
	}
}

// sdh 27/6/2002: box function for drawing filled boxes

void Vid_Box(int x, int y, int w, int h, int c)
{
	unsigned char *p = vid_vram + (SCR_HGHT-1-y) * vid_pitch + x;

	for (; h >= 0; --h, p += vid_pitch)
		memset(p, c, w);
}

//
// Clear screen
//

void Vid_ClearBuf(void)
{
	memset(vid_vram, 0, VRAMSIZE);
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/7/2002: remove collision detection code
// sdh 27/6/2002: move to new sopsym_t for symbols
// sdh 25/04/2002: rename vga_{pitch,vram} to vid_{pitch,vram}
// sdh 26/03/2002: split off platform specific drawing functions here
//                 replaced amiga drawing functions with these generic
//                 8 bit ones
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
