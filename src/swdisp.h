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

#ifndef __SWDISP_H__
#define __SWDISP_H__

#include "sw.h"

extern void dispplyr(OBJECTS *ob);
extern void dispbomb(OBJECTS *obp);
extern void dispmiss(OBJECTS *obp);
extern void dispburst(OBJECTS *obp);
extern void dispexpl(OBJECTS *obp);
extern void dispcomp(OBJECTS *ob);
extern void disptarg(OBJECTS *ob);
extern void dispflck(OBJECTS *ob);
extern void dispbird(OBJECTS *ob);

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.7  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.6  2005/04/29 18:57:12  fraggle
// Move dispscore and medal drawing code into swstbar.c
//
// Revision 1.5  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.4  2004/10/15 18:51:24  fraggle
// Fix the map. Rename dispworld to dispmap as this is what it really does.
//
// Revision 1.3  2004/10/15 17:52:31  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.2  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.1.1.1  2003/02/14 19:03:30  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

