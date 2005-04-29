// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
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
// CGA Video Interface
//
//-----------------------------------------------------------------------

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "sw.h"

typedef enum {
	KEY_UNKNOWN,
	KEY_PULLUP,
	KEY_PULLDOWN,
	KEY_FLIP,
	KEY_BOMB,
	KEY_FIRE,
	KEY_HOME,
	KEY_MISSILE,
	KEY_STARBURST,
	KEY_ACCEL,
	KEY_DECEL,
	KEY_SOUND,
	NUM_KEYS,
} sopkey_t;

// which keys are currently down
// this is actually a simple bitfield
// bit 0 is whether the button is currently down
// bit 1 is whether the button has been pressed
//       since the last call of Vid_GetGameKeys
// in this way, every button press will have an effect:
// if it is done based on what is currently down it is
// possible to miss keypresses (if you press and release
// a button fast enough)

extern int keysdown[NUM_KEYS];

extern BOOL vid_fullscreen;         // fullscreen
extern BOOL vid_double_size;        // x2 scale

// init/shutdown

extern void Vid_Init();
extern void Vid_Shutdown();
extern void Vid_Reset();

// update screen

extern void Vid_Update();

extern BOOL Vid_GetCtrlBreak();

// keyboard functions

extern int Vid_GetKey();
extern int Vid_GetGameKeys();

// drawing routines

void Vid_Box(int x, int y, int w, int h, int c);

// draw ground

extern void Vid_DispGround(GRNDTYPE *gptr);
extern void Vid_DispGround_Solid(GRNDTYPE *gptr);

// draw a pixel

extern void Vid_PlotPixel(int x, int y, int clr);
extern void Vid_XorPixel(int x, int y, int clr);

extern int Vid_GetPixel(int x, int y);

// draw a symbol

extern void Vid_DispSymbol(int x, int y, sopsym_t *symbol,
			   int clr);

// clear screen

void Vid_ClearBuf();

#endif

//-----------------------------------------------------------------------
//
// $Log$
// Revision 1.5  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.4  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.3  2003/06/08 03:41:42  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.2.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.2  2003/03/26 13:53:29  fraggle
// Allow control via arrow keys
// Some code restructuring, system-independent video.c added
//
// Revision 1.1.1.1  2003/02/14 19:03:34  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/07/2002: removed retcode from Vid_DispSymbol
// sdh 28/06/2002: remove redundant object reference from dispsymbol
// sdh 27/06/2002: move to new sopsym_t for symbols
// sdh 27/03/2002: split disppixel to several functions
// sdh 26/03/2002: add drawing function, rename video.h
// sdh 21/10/2001: added cvs tags
//
//-----------------------------------------------------------------------
