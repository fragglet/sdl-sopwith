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

#ifndef __SWINIT_H__
#define __SWINIT_H__

#include "sw.h"

extern void swinit(int argc, char *argv[]);
extern void initseed();
extern void swrestart();
extern void initgdep();
extern void initdisp(BOOL reset);
extern void dispcgge(OBJECTS *ob);
extern void dispfgge(OBJECTS *ob);
extern void dispbgge(OBJECTS *ob);
extern void dispsgge(OBJECTS *ob);
extern void dispmgge(OBJECTS *ob);
extern void dispsbgge(OBJECTS *ob);
extern void initcomp(OBJECTS *obp);
extern void initplyr(OBJECTS *obp);
extern OBJECTS *initpln(OBJECTS *obp);
extern void initshot(OBJECTS *obop, OBJECTS *targ);
extern void initbomb(OBJECTS *obop);
extern void initmiss(OBJECTS *obop);
extern void initburst(OBJECTS *obop);
extern void initexpl(OBJECTS *obop, int small);
extern void initsmok(OBJECTS *obop);
extern void initflck();
extern void initbird(OBJECTS *obop, int i);
extern void initoxen();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

