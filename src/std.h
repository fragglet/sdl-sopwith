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

#ifndef __STD_H__
#define __STD_H__

#include <stdio.h>
#include <stdlib.h>

// sdh 21/10/2001: moved BOOL here from sw.h, made into enum

typedef enum {FALSE, TRUE} BOOL;
enum {NO, YES};

#endif

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.3  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.2  2003/04/05 22:55:11  fraggle
// Remove the FOREVER macro and some unused stuff from std.h
//
// Revision 1.1.1.1  2003/02/14 19:03:28  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: rearranged headers, added cvs tags
//
//---------------------------------------------------------------------------

