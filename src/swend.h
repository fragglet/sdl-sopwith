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

#ifndef __SWEND_H__
#define __SWEND_H__

#include "sw.h"

extern void swend(char *msg, BOOL update);
extern void endgame(int targclr);
extern void winner(OBJECTS *obp);
extern void loser(OBJECTS *ob);
extern void swreport();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

