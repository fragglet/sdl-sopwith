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

#ifndef __SWMAIN_H__
#define __SWMAIN_H__

#include <setjmp.h>

#include "sw.h"

extern int     playmode;
extern GAMES   *currgame;
extern OBJECTS *targets[MAX_TARG+MAX_OXEN];
extern int     numtarg[2];
extern int     savemode;
extern int     tickmode;
extern int     counttick, countmove;
extern int     movetick, movemax;
extern int     gamenum;
extern int     gmaxspeed, gminspeed;
extern int     targrnge;
extern char    auxdisp[0x8000];
extern int     multkey;
extern int     multtick;
extern BOOL    hires;
extern BOOL    disppos;
extern BOOL    titleflg;
extern int     dispdbg;
extern BOOL    soundflg;
extern BOOL    repflag;
extern BOOL    joystick;
extern BOOL    ibmkeybd;
extern BOOL    inplay;
extern BOOL    printflg;
extern int     koveride;
extern int     missok;
extern int     displx, disprx;
extern int     dispdx;
extern BOOL    dispinit;
extern OBJECTS *drawlist;
extern OBJECTS *nobjects;
extern OBJECTS oobjects[MAX_PLYR];
extern OBJECTS *objbot, *objtop, *objfree, *deltop, *delbot;
extern OBJECTS topobj, botobj;
extern OBJECTS *compnear[MAX_PLYR];
extern int     lcompter[MAX_PLYR];
extern int     rcompter[MAX_PLYR];
extern OBJECTS *objsmax;
extern int     endsts[MAX_PLYR];
extern int     endcount;
extern int     player;
extern int     currobx;
extern BOOL    plyrplane;
extern BOOL    compplane;
extern BOOL    goingsun;
extern BOOL    forcdisp;
extern char    *histin, *histout;
extern unsigned explseed;
extern int     keydelay;
extern int     dispcnt;
extern int     endstat;
extern int     maxcrash;
extern int     shothole;
extern int     splatbird;
extern int     splatox;
extern int     oxsplatted;
extern int     sintab[ANGLES];
extern MULTIO  *multbuff;
extern OLDWDISP wdisp[MAX_OBJS];
extern jmp_buf envrestart;

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 24/10/2001: fix auxdisp buffer
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

