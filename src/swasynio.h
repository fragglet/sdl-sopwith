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

#ifndef __SWASYNIO_H__
#define __SWASYNIO_H__

#include "sw.h"

typedef enum { ASYN_LISTEN, ASYN_CONNECT, ASYN_TCPLOOP } asynmode_t;

extern asynmode_t asynmode;
extern char asynhost[128];

extern void asynput(int movekey);
extern char *asynclos(BOOL update);
extern void init1asy();
extern void init2asy();
extern void asynupdate(void);

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.5  2004/10/20 18:56:11  fraggle
// Remove prototype for removed function
//
// Revision 1.4  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.3  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.2.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.2  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.1.1.1  2003/02/14 19:03:29  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

