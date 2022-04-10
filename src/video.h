// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
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
// CGA Video Interface
//
//-----------------------------------------------------------------------

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "sw.h"

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

// draw to the screen

void Vid_SetBuf();

// clear screen

void Vid_ClearBuf();

// draw to aux buffer

void Vid_SetBuf_Aux();

// clear aux buffer

void Vid_ClearBuf_Aux();

// copy bottom of aux buffer to screen

void Vid_CopyBuf();


#endif

//-----------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:34  fraggle
// Initial revision
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
