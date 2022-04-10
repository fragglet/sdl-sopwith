// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001 Simon Howard
//
// All rights reserved except as specified in the file license.txt.
// Distribution of this file without the license.txt file accompanying
// is prohibited.
//
//---------------------------------------------------------------------------

#ifndef __SWPLANES_H__
#define __SWPLANES_H__

#include "sw.h"

#define HITSYMS         2               /*  Number of hit symbols per plane */
#define SYMBYTES        64              /*  Bytes in a symbol               */
#define WINSIZES        4               /*  Number of winner plane sizes    */
#define WINBYTES        64              /*  Bytes in a winner symbol        */

extern char    swplnsym[ORIENTS][ANGLES][SYMBYTES];
extern char    swhitsym[HITSYMS][SYMBYTES];
extern char    swwinsym[WINSIZES][WINBYTES];

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:02:47  fraggle
// Initial revision
//
//
// sdh 21/10/2001: moved plane sprite constants into here from sw.h
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

