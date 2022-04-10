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
//  Graphics backend for 8-bit (VGA) displays
//
//---------------------------------------------------------------------------

#include "video.h"

#include "sw.h"
#include "swdisp.h"
#include "swground.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swsymbol.h"
#include "swutil.h"

// this should be in a header somewhere
#define SBAR_HGHT 19

#define VRAMSIZE (SCR_HGHT * vid_pitch)

static char *dispoff = NULL;           // current display offset

unsigned char *vid_vram;
unsigned int vid_pitch;

// sdh 28/10/2001: moved auxdisp here

char *auxdisp = NULL;

/*---------------------------------------------------------------------------

        Update display of ground.   Delete previous display of ground by
        XOR graphics.

        Different routines are used to display/delete ground on colour
        or monochrome systems.

---------------------------------------------------------------------------*/


void Vid_DispGround(GRNDTYPE *gptr)
{
	register GRNDTYPE *g = gptr;
	register unsigned char *sptr;
	register int x, y;
	register int gc, gl;

	gl = *g;

	sptr = dispoff + (SCR_HGHT-1 - gl) * vid_pitch;

	for (x=SCR_WDTH, g = gptr; x>0; --x) {
		gc = *g++;
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
	register GRNDTYPE *g = gptr;
	register unsigned char *sptr;
	register int x, y;
	register int gc, gl;

	gl = *g;

	for (x=0, g = gptr; x<SCR_WDTH; ++x) {
		gc = *g++;

		sptr = dispoff + (SCR_HGHT-SBAR_HGHT-1) * vid_pitch + x;
	
		for (y = gc-SBAR_HGHT+1; y; --y) {
			*sptr ^= 3;
			sptr -= vid_pitch;
		}
	}
}


/*---------------------------------------------------------------------------

        External calls to display a point of a specified colour at a
        specified position.   The point request may or may not ask for
        collision detection by returning the old colour of the point.

        Different routines are used to display points on colour or
        monochrome systems.

---------------------------------------------------------------------------*/

void Vid_PlotPixel(int x, int y, int clr)
{
	register unsigned char *sptr 
		= dispoff + (SCR_HGHT-1 - y) * vid_pitch + x;

	*sptr = clr & 3;
}

void Vid_XorPixel(int x, int y, int clr)
{
	register unsigned char *sptr 
		= dispoff + (SCR_HGHT-1 - y) * vid_pitch + x;

	*sptr ^= clr & 3;
}

int Vid_GetPixel(int x, int y)
{
	register unsigned char *sptr 
		= dispoff + (SCR_HGHT-1 - y) * vid_pitch + x;

	return *sptr;
}


/*---------------------------------------------------------------------------

        Display an object's current symbol at a specified screen location
        Collision detection may or may not be asked for.

        Different routines are used to display symbols on colour or
        monochrome systems.

---------------------------------------------------------------------------*/

// sdh 27/7/2002: removed collision detection, this is now done
// independently of the drawing code (retcode)

void Vid_DispSymbol(int x, int y, sopsym_t *symbol, int clr)
{
	unsigned char *sptr = dispoff + (SCR_HGHT-1 - y) * vid_pitch + x;
	unsigned char *data = symbol->data;
	int x1, y1;
	int w = symbol->w, h = symbol->h;
	int wrap = x - SCR_WDTH + w; 

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

	if (h > y + 1)
		h = y + 1;

	if (clr == 2) {
		for (y1=0; y1<h; ++y1) {
			unsigned char *sptr2 = sptr;
			for (x1=0; x1<w; ++x1, ++sptr2) {
				int i = *data++;

				if (i)
					*sptr2 ^= i ^ 3;
			}
			data += wrap;
			sptr += vid_pitch;
		}
	} else {
		for (y1=0; y1<h; ++y1) {
			unsigned char *sptr2 = sptr;
			for (x1=0; x1<w; ++x1, ++sptr2) {
				unsigned int i = *data++;

				if (i)
					*sptr2 ^= i;
			}
			data += wrap;
			sptr += vid_pitch;
		}
	}
}

// sdh 27/6/2002: box function for drawing filled boxes

void Vid_Box(int x, int y, int w, int h, int c)
{
	unsigned char *p = dispoff + (SCR_HGHT-1-y) * vid_pitch + x;

	for (; h >= 0; --h, p += vid_pitch)
		memset(p, c, w);
}

/*---------------------------------------------------------------------------

        External calls to specify current video ram as screen ram or
        auxiliary screen area.

---------------------------------------------------------------------------*/

void Vid_SetBuf()
{
	dispoff = vid_vram;
}

void Vid_SetBuf_Aux()
{
	if (!auxdisp)
		auxdisp = malloc(VRAMSIZE);

	dispoff = auxdisp;
}

// sdh 28/10/2001: moved various auxdisp functions here:

void Vid_CopyBuf()
{
	memset(vid_vram, 0, VRAMSIZE);
	memcpy(vid_vram + ((SCR_HGHT-SBAR_HGHT) * vid_pitch), 
	       auxdisp + ((SCR_HGHT-SBAR_HGHT) * vid_pitch), 
	       SBAR_HGHT*vid_pitch);
}


void Vid_ClearBuf()
{
	memset(vid_vram, 0, VRAMSIZE);
}

void Vid_ClearBuf_Aux()
{
	if (!auxdisp)
		auxdisp = malloc(VRAMSIZE);

	memset(auxdisp, 0, VRAMSIZE);
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:34  fraggle
// Initial revision
//
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
//---------------------------------------------------------------------------

