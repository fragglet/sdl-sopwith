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

#ifndef __SWAUTO_H__
#define __SWAUTO_H__

#include "sw.h"

extern void swauto(OBJECTS *ob);
extern void attack(OBJECTS *obp, OBJECTS *obt);
extern int gohome(OBJECTS *obpt);
extern int shoot(OBJECTS *obt);
extern int aim(OBJECTS *obo, int ax, int ay, OBJECTS *obt, BOOL longway);
extern int range(int x, int y, int ax, int ay);

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

