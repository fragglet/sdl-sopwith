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

#ifndef __SWSOUND_H__
#define __SWSOUND_H__

#include "sw.h"

#define S_TITLE         05
#define S_EXPLOSION     10              /*  Sound priorities                */
#define S_BOMB          20
#define S_SHOT          30
#define S_FALLING       40
#define S_HIT           50
#define S_PLANE         60

extern void initsndt();
extern void sound(int type, int parm, OBJECTS *ob);
extern void swsound();
extern void soundadj();
extern void initsound(OBJECTS *obp, int type);
extern void stopsound(OBJECTS *ob);
extern OBJECTS *ob;
extern unsigned freq;
extern unsigned  modulo;
extern void swsndupdate();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: moved sound priorities into here from sw.h
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

