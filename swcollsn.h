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

#ifndef __SWCOLLSN_H__
#define __SWCOLLSN_H__

#include "sw.h"

extern void swcollsn();
extern void colltest(OBJECTS *ob1, OBJECTS *ob2);
extern void tstcrash(OBJECTS *obp);
extern void swkill(OBJECTS *ob1, OBJECTS *ob2);
extern void scorepln(OBJECTS *ob);
extern void dispscore(OBJECTS *obp);
extern void dispd(int n, int size);
extern BOOL equal(int (*x)(), int (*y)() );

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

