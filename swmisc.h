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

#ifndef __SWMISC_H__
#define __SWMISC_H__

#include "sw.h"

extern void swputc(char c);
extern void swputs( char *sp );
extern void swgets(char *s, int max);
extern void swcolour(int a);
extern void swposcur(int a, int b);
extern int swgetc();
extern void swflush();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

