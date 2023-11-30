//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//

#ifndef __SWMAIN_H__
#define __SWMAIN_H__

#include <setjmp.h>

#include "sw.h"

/* maximum number of tics before the game stops sending commands over
 * the network */

#define MAX_NET_LAG 12

extern bool conf_missiles;
extern bool conf_solidground;
extern bool conf_hudsplats;
extern bool conf_wounded;
extern bool conf_animals;
extern bool conf_harrykeys;
extern bool conf_medals;
extern bool conf_big_explosions;
extern int conf_video_palette;

extern OBJECTS *consoleplayer;
extern playmode_t playmode;
extern const GAMES *currgame;
extern int     numtarg[2];
extern int     counttick, countmove;
extern int     gamenum;
extern int     initial_gamenum;
extern int     gmaxspeed, gminspeed;
extern int     targrnge;
extern bool    titleflg;
extern bool    soundflg;
extern int     displx;
extern OBJECTS *planes[MAX_PLYR];
extern int     num_planes;
extern OBJECTS *objbot, *objtop, *objfree, *deltop, *delbot;
extern OBJECTS topobj, botobj;
extern OBJECTS *objsmax;
extern int     endcount;
extern int     player;
extern bool    plyrplane;
extern bool    compplane;
extern bool    forcdisp;
extern char    *histin, *histout;
extern unsigned explseed;
extern int     keydelay;
extern int     dispcnt;
extern int     endstat;
extern int     maxcrash;
extern jmp_buf envrestart;

extern int latest_player_commands[MAX_PLYR][MAX_NET_LAG];
extern int latest_player_time[MAX_PLYR];
extern int num_players;

extern int swmain(int argc, char *argv[]);

#endif
