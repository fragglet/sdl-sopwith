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

#ifndef __SWASYNIO_H__
#define __SWASYNIO_H__

#include "sw.h"

typedef enum { ASYN_LISTEN, ASYN_CONNECT } asynmode_t;

extern asynmode_t asynmode;
extern char asynhost[128];

extern int asynget(OBJECTS *ob);
extern void asynput();
extern char    *asynclos(BOOL update);
extern void asyninit();
extern void init1asy();
extern void init2asy();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

