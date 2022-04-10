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

#ifndef __SWNETIO_H__
#define __SWNETIO_H__

#include "sw.h"

extern int *multstack;
extern char *multfile;
extern int _word(char *ptr);
extern int _byte(char *ptr);
extern void _fromintel();
extern char *_tointel();
extern BOOL movemult(OBJECTS *obp);
extern void multopen(char *cmndfile, char *multfile);
extern void multparm(char *multfile);
extern void sectparm();
extern int multread();
extern int multwrite();
extern int multunlock();
extern void multwait();
extern int multget(OBJECTS *ob);
extern void multput();
extern char *multclos(BOOL update);
extern void delaymov();
extern void changedelay();
extern void delayrep();
extern void statrep();
extern int editnum();
extern void init1mul(BOOL reset, char *device);
extern void init2mul();

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:31  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

