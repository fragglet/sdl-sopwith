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

#ifndef __SWDISP_H__
#define __SWDISP_H__

#include "sw.h"

extern void dispplyr(OBJECTS *ob);
extern void dispbomb(OBJECTS *obp);
extern void dispmiss(OBJECTS *obp);
extern void dispburst(OBJECTS *obp);
extern void dispexpl(OBJECTS *obp);
extern void dispcomp(OBJECTS *ob);
extern void dispmult(OBJECTS *ob);
extern void disptarg(OBJECTS *ob);
extern void dispflck(OBJECTS *ob);
extern void dispbird(OBJECTS *ob);
extern void dispwobj(OBJECTS *obp);
extern unsigned long   randsd();
extern void dispwindshot();
extern void dispsplatbird();
extern void dispoxsplat();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

