// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2003 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------------

#ifndef __SWMAIN_H__
#define __SWMAIN_H__

#include <setjmp.h>

#include "sw.h"

extern BOOL conf_missiles;
extern BOOL conf_solidground;
extern BOOL conf_hudsplats;
extern BOOL conf_wounded;
extern BOOL conf_animals;
extern BOOL conf_harrykeys;

extern playmode_t playmode;
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
// $Log$
// Revision 1.1  2003/02/14 19:03:31  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 28/10/2001: conf_ game options
// sdh 28/10/2001: moved auxdisp buffer to swgrpha.c
// sdh 24/10/2001: fix auxdisp buffer
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

