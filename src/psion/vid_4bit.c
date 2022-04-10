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
// 4-bit graphics routines for the psion screen
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
unsigned int vid_pitch;              // line length in bytes(not pixels)

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
	char *p = dispoff + (SCR_HGHT-1 - *gptr) * vid_pitch;
	int oldy = *gptr;
	int x;

	for(x=SCR_WDTH; x; ++p) {
		if (oldy == *gptr) {
			*p ^= 0xf;
			p[vid_pitch] ^= 0xf;
		} else if (oldy < *gptr) {
			for(; oldy < *gptr; ++oldy) {
				*p ^= 0xf;
				p -= vid_pitch;
			}
		} else if (oldy > *gptr) {
			for (; oldy > *gptr; --oldy) {
				*p ^= 0xf;
				p += vid_pitch;
			}
		}

		++gptr; --x;

		if (oldy == *gptr) {
			*p ^= 0xf0;
			p[vid_pitch] ^= 0xf0;
		} else if (oldy < *gptr) {
			for(; oldy < *gptr; ++oldy) {
				*p ^= 0xf0;
				p -= vid_pitch;
			}
		} else if (oldy > *gptr) {
			for (; oldy > *gptr; --oldy) {
				*p ^= 0xf0;
				p += vid_pitch;
			}
		}

		++gptr; --x; 
	}
}


// sdh 28/10/2001: solid ground function

void Vid_DispGround_Solid(GRNDTYPE * gptr)
{
	char *p = dispoff + (SCR_HGHT-SBAR_HGHT-1) * vid_pitch;
	int x;

	for (x=0; x<SCR_WDTH; ++p) {
		char *p2;
		int i;

		for (i=*gptr+1 - SBAR_HGHT, p2=p; i; --i) {
			*p2 ^= 0xf;
			p2 -= vid_pitch;
		}

		++x; ++gptr;

		for (i=*gptr+1 - SBAR_HGHT, p2=p; i; --i) {
			*p2 ^= 0xf0;
			p2 -= vid_pitch;
		}

		++x; ++gptr;
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
	unsigned char *s = dispoff + (SCR_HGHT-1-y) * vid_pitch + (x >> 1);

	clr = ~clr & 0x3;
	clr |= clr << 2;

	if (x & 1) {
		*s = (*s & 0xf) | (clr << 4);
	} else {
		*s = (*s & 0xf0) | clr;
	}
}

void Vid_XorPixel(int x, int y, int clr)
{
	unsigned char *s = dispoff + (SCR_HGHT-1-y) * vid_pitch + (x >> 1);

	clr &= 0x3;
	clr |= clr << 2;

	if (x & 1)
		*s ^= clr << 4;
	else
		*s ^= clr;
}

int Vid_GetPixel(int x, int y)
{
	unsigned char *s = dispoff + (SCR_HGHT-1-y) * vid_pitch + (x >> 1);

	if (x & 1)
		return (~*s >> 6) & 3;
	else
		return (~*s >> 2) & 3;
}


/*---------------------------------------------------------------------------

        Display an object's current symbol at a specified screen location
        Collision detection may or may not be asked for.

        Different routines are used to display symbols on colour or
        monochrome systems.

---------------------------------------------------------------------------*/

// sdh 28/6/2002: move to new sopsym_t for sprites
// sdh 27/7/2002: removed collision detection code, this is now done
// independently of the drawing code (retcode)

void Vid_DispSymbol(int x, int y, sopsym_t *symbol, int clr)
{
	unsigned char *s = dispoff + (SCR_HGHT-1-y) * vid_pitch + (x >> 1);
	unsigned char *data = symbol->data;
	int w, h;

	if (symbol->h == 1 && symbol->w == 1) {
		Vid_XorPixel(x, y, clr);
		return;
	}

	w = symbol->w;
	h = symbol->h;

	if (h > y) 
		h = y;

	if (clr == 2) {

		// inverted colour draw (for enemy planes)

		for (y=0; y<h; ++y) {
			unsigned char *s2 = s;
	
			for (x=0; x<w; x += 2) {
				int i;

				i = *data++;
       
				if (i)
					*s2 ^= (i | (i << 2)) ^ 0xf;

				i = *data++;
	
				if (i)
					*s2 ^= ((i << 6) | (i << 4)) ^ 0xf0;
	
				++s2;
			}
	
			s += vid_pitch;
		}
	} else {

		// normal draw

		for (y=0; y<h; ++y) {
			unsigned char *s2 = s;

			for (x=0; x<w; x += 2) {
				int i;
	
				i = *data++;
	
				if (i)
					*s2 ^= (i << 2) | i;
	
				i = *data++;
	
				if (i)
					*s2 ^= (i << 6) | (i << 4);
	
				++s2;
			}
	
			s += vid_pitch;
		}
	}
}

// sdh 28/6/2002: new function to draw filled boxes

void Vid_Box(int x, int y, int w, int h, int c)
{
	unsigned char *s = dispoff + (SCR_HGHT-1-y) * vid_pitch + (x >> 1);
	
	c = ~c & 0x3;
	c |= (c << 6) | (c << 4) | (c << 2);
	w >>= 1;

	for (; h >= 0; --h, s += vid_pitch)
		memset(s, c, w);
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
	memset(vid_vram, 0xff, VRAMSIZE);
	memcpy(vid_vram + ((SCR_HGHT-SBAR_HGHT) * vid_pitch), 
	       auxdisp + ((SCR_HGHT-SBAR_HGHT) * vid_pitch), 
	       SBAR_HGHT*vid_pitch);
}


void Vid_ClearBuf()
{
	memset(vid_vram, 0xff, VRAMSIZE);
}

void Vid_ClearBuf_Aux()
{
	if (!auxdisp)
		auxdisp = malloc(VRAMSIZE);

	memset(auxdisp, 0xff, VRAMSIZE);
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:39  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/7/2002: remove collision detection code
// sdh 28/6/2002: move to sopsym_t for sprites
// sdh 25/04/2002: rename vid4_{pitch,vram} to vid_{pitch,vram}
// sdh 20/04/2002: psion framebuffer port
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

