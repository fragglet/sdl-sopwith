// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------

#ifndef __SWMOVE_H__
#define __SWMOVE_H__

#include "sw.h"

extern void swmove();
extern BOOL moveplyr(OBJECTS *obp);
extern void interpret(OBJECTS *obp, int key);
extern BOOL movecomp(OBJECTS *obp);
extern BOOL movepln(OBJECTS *obp);
extern BOOL moveshot(OBJECTS *obp);
extern BOOL movebomb(OBJECTS *obp);
extern BOOL movemiss(OBJECTS *obp);
extern BOOL moveburst(OBJECTS *obp);
extern BOOL movetarg(OBJECTS *obt);
extern BOOL moveexpl(OBJECTS *obp);
extern BOOL movesmok(OBJECTS *obp);
extern BOOL moveflck(OBJECTS *obp);
extern BOOL movebird(OBJECTS *obp);
extern BOOL moveox(OBJECTS *ob);
extern BOOL crashpln(OBJECTS *obp);
extern BOOL hitpln(OBJECTS *obp);
extern BOOL insertx(OBJECTS *ob, OBJECTS *obp);
extern void deletex(OBJECTS *obp);

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.3  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.2  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.1.1.1  2003/02/14 19:03:31  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

