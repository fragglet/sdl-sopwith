// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//
// Converted ASM Routines
// Some of these arent used (dos specific empty functions)
// Some of them are modified from andrew jenners source functions
// as i cant read x86 asm :(
//
//-----------------------------------------------------------------------

#include "sw.h"
#include "swutil.h"

void movexy(OBJECTS * ob, int *x, int *y)
{
	long pos;
	//long vel;
//      pos = (((long) (ob->ob_x)) << 16) + ob->ob_lx;
//      vel = (((long) (ob->ob_dx)) << 16) + ob->ob_ldx;
	pos = (ob->ob_x + ob->ob_dx) << 16;
	ob->ob_x = (short) (pos >> 16);
	ob->ob_lx = (short) pos;
	*x = ob->ob_x;
//      pos = (((long) (ob->ob_y)) << 16) + ob->ob_ly;
//      vel = (((long) (ob->ob_dy)) << 16) + ob->ob_ldy;
	pos = (ob->ob_y + ob->ob_dy) << 16;
	ob->ob_y = (short) (pos >> 16);
	ob->ob_ly = (short) pos;
	*y = ob->ob_y;
}

void setdxdy(OBJECTS * obj, int dx, int dy)
{
	obj->ob_dx = dx >> 8;
	obj->ob_ldx = dx << 8;
	obj->ob_dy = dy >> 8;
	obj->ob_ldy = dy << 8;
}

void swsetblk()
{
	// used by the splatted ox code to colour the screen. ?
}

void swgetjoy()
{
	// joystick
}

void histend()
{
	// demos?
}


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.2  2003/06/04 16:02:55  fraggle
// Remove broken printscreen function
//
// Revision 1.1.1.1  2003/02/14 19:03:22  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 16/11/2001: trap14 removed
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 18/10/2001: converted all functions to ANSI-style arguments
// sdh ??/10/2001: created this file to hold replacements for the ASM
//                 functions in swutil.asm
//
//---------------------------------------------------------------------------
