// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001 Simon Howard
//
// All rights reserved except as specified in the file license.txt.
// Distribution of this file without the license.txt file accompanying
// is prohibited.
//
//---------------------------------------------------------------------------

#ifndef __SWUTIL_H__
#define __SWUTIL_H__

#include "sw.h"

extern void movexy(OBJECTS *ob, int *x, int *y);
extern void setdxdy(OBJECTS *obj, int dx, int dy);
extern void trap14();
extern void swsetblk();
extern void swprint();
extern void swgetjoy();
extern void histend();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

