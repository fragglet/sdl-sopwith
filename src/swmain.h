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

/* maximum number of tics before the game stops sending commands over 
 * the network */

#define MAX_NET_LAG 12

extern BOOL conf_missiles;
extern BOOL conf_solidground;
extern BOOL conf_hudsplats;
extern BOOL conf_wounded;
extern BOOL conf_animals;
extern BOOL conf_harrykeys;

extern OBJECTS *consoleplayer;
extern playmode_t playmode;
extern GAMES   *currgame;
extern OBJECTS *targets[MAX_TARG+MAX_OXEN];
extern int     numtarg[2];
extern int     savemode;
extern int     tickmode;
extern int     counttick, countmove;
extern int     gamenum;
extern int     gmaxspeed, gminspeed;
extern int     targrnge;
extern BOOL    disppos;
extern BOOL    titleflg;
extern int     dispdbg;
extern BOOL    soundflg;
extern BOOL    repflag;
extern BOOL    inplay;
extern int     displx;
extern OBJECTS *nobjects;
extern OBJECTS oobjects[MAX_PLYR];
extern OBJECTS *objbot, *objtop, *objfree, *deltop, *delbot;
extern OBJECTS topobj, botobj;
extern OBJECTS *compnear[MAX_PLYR];
extern int     lcompter[MAX_PLYR];
extern int     rcompter[MAX_PLYR];
extern OBJECTS *objsmax;
extern int     endcount;
extern int     player;
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
extern int     sintab[ANGLES];
extern jmp_buf envrestart;

extern int latest_player_commands[MAX_PLYR][MAX_NET_LAG];
extern int latest_player_time[MAX_PLYR];
extern int num_players;

extern int swmain(int argc, char *argv[]);

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.13  2004/10/20 19:00:01  fraggle
// Remove currobx, endsts variables
//
// Revision 1.12  2004/10/15 22:28:39  fraggle
// Remove some dead variables and code
//
// Revision 1.11  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.10  2004/10/15 18:57:14  fraggle
// Remove redundant wdisp variable
//
// Revision 1.9  2004/10/15 17:23:32  fraggle
// Restore HUD splats
//
// Revision 1.8  2004/10/14 08:48:46  fraggle
// Wrap the main function in system-specific code.  Remove g_argc/g_argv.
// Fix crash when unable to initialise video subsystem.
//
// Revision 1.7  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.6  2003/06/08 02:48:45  fraggle
// Remove dispdx, always calculated displx from the current player position
// and do proper edge-of-level bounds checking
//
// Revision 1.5  2003/06/08 02:39:25  fraggle
// Initial code to remove XOR based drawing
//
// Revision 1.4.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.4  2003/06/04 17:13:26  fraggle
// Remove disprx, as it is implied from displx anyway.
//
// Revision 1.3  2003/06/04 16:02:55  fraggle
// Remove broken printscreen function
//
// Revision 1.2  2003/04/05 22:31:29  fraggle
// Remove PLAYMODE_MULTIPLE and swnetio.c
//
// Revision 1.1.1.1  2003/02/14 19:03:31  fraggle
// Initial Sourceforge CVS import
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

