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

#ifndef __SWNETIO_H__
#define __SWNETIO_H__

#include "sw.h"

extern int *multstack;
extern char *multfile;
extern int _word(char *ptr);
extern int _byte(char *ptr);
extern void _fromintel();
extern char *_tointel();
extern BOOL movemult(OBJECTS *obp);
extern void multopen(char *cmndfile, char *multfile);
extern void multparm(char *multfile);
extern void sectparm();
extern int multread();
extern int multwrite();
extern int multunlock();
extern void multwait();
extern int multget(OBJECTS *ob);
extern void multput();
extern char *multclos(BOOL update);
extern void delaymov();
extern void changedelay();
extern void delayrep();
extern void statrep();
extern int editnum();
extern void init1mul(BOOL reset, char *device);
extern void init2mul();

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

