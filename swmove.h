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
extern BOOL stallpln(OBJECTS *obp);
extern BOOL insertx(OBJECTS *ob, OBJECTS *obp);
extern void deletex(OBJECTS *obp);

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

