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
//
//        swmain   -      SW mainline
//

#include "timer.h"
#include "video.h"

#include "sw.h"
#include "swasynio.h"
#include "swcollsn.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swmove.h"
#include "swsound.h"
#include "swtitle.h"

// sdh 28/10/2001: game options

bool conf_missiles = 0;             // allow missiles: replaces missok
bool conf_solidground = 0;          // draw ground solid like in sopwith 1
bool conf_hudsplats = 0;            // splatted birds etc
bool conf_wounded = 0;              // enable wounded planes
bool conf_animals = 1;              // birds and oxen
bool conf_harrykeys = 0;            // plane rotation relative to screen
bool conf_medals = 1;
bool conf_big_explosions = 1;       // big oil tank explosions

int conf_video_palette = 0;			// Video palette selection (0 is the default CGA color scheme)

playmode_t playmode;		/* Mode of play                     */
const GAMES *currgame;		/* Game parameters and current game */
OBJECTS *consoleplayer;
int numtarg[2];			/* Number of active targets by color */
int countmove;			/* Performance counters             */

int gamenum = 0;		/* Current game number              */
int gmaxspeed, gminspeed;	/* Speed range based on game number */
int targrnge;			/* Target range based on game number */

bool titleflg;			/* Title flag                       */
bool soundflg = 0;		/* Sound flag                       */

int displx;			/* Display left and right           */

OBJECTS *planes[MAX_PLYR];      /* Plane objects                    */
int num_planes;

OBJECTS *objbot, *objtop,	/* Top and bottom of object list    */
*objfree,			/* Free list                        */
*deltop, *delbot;		/* Newly deallocated objects        */
OBJECTS topobj, botobj;		/* Top and Bottom of obj. x list    */

int endcount;
int player;			/* Pointer to player's object       */
bool plyrplane;			/* Current object is player flag    */
bool compplane;			/* Current object is a comp plane   */
unsigned explseed;		/* random seed for explosion        */

int keydelay = -1;		/* Number of displays per keystroke */
int dispcnt;			/* Displays to delay keyboard       */
int endstat;			/* End of game status for curr. move */
int maxcrash;			/* Maximum number of crashes        */

const int sintab[ANGLES] = {	/* sine table of pi/8 increments    */
	0, 98, 181, 237,	/*   multiplied by 256              */
	256, 237, 181, 98,
	0, -98, -181, -237,
	-256, -237, -181, -98
};

jmp_buf envrestart;		/* Restart environment for restart  */
				/*  long jump.                      */

/* player commands */

/* buffer of player commands, loops round.
 * latest_player_commands[plr][latest_player_time[plr] % MAX_NET_LAG] is the
 * very latest command for plr.
 */

int latest_player_commands[MAX_PLYR][MAX_NET_LAG];
int latest_player_time[MAX_PLYR];
int num_players;

/* Time each player command in the buffer was created.
 * We store this to calculate the lag between the player command
 * being created and the command being executed. */

static int player_command_time[MAX_NET_LAG];

/* Skip time.  This is used to keep players in sync.
 * Each player waits a slight bit longer than they would normally
 * (ie. in a single player game): the amount equal to skip_time here.
 * skip_time is generated from the lag players experience.
 * This means that lagged players wait a bit to "catch up" with the
 * others, keeping the game in sync.
 */

static int skip_time;

/* possibly advance the game */

static int can_move(void)
{
	int i;
	int lowtic = countmove + MAX_NET_LAG;

	/* we can only advance the game if latest_player_time for all
	 * players is > countmove. */

	for (i=0; i<num_players; ++i) {
		if (latest_player_time[i] < lowtic) {
			lowtic = latest_player_time[i];
		}
	}

	return lowtic > countmove;
}

/* Calculate lag between the controls and the game */

static void calculate_lag(void)
{
	int lag = Timer_GetMS() - player_command_time[countmove % MAX_NET_LAG];
	int compensation;

	// only make a small adjustment based on the lag, so as not
	// to affect the playability.  however, over a long period
	// this should have the desired effect.

	compensation = lag / 100;

	// bound the compensation applied; responds to network traffic
	// spikes

	if (compensation < -5) {
		compensation = -5;
	} else if (compensation > 5) {
		compensation = 5;
	}

	skip_time += compensation;
}

static void new_move(void)
{
	int multkey;
	int tictime;

	/* generate a new move command and save it */

	multkey = Vid_GetGameKeys();
	if (conf_harrykeys) {
		multkey |= K_HARRYKEYS;
	}

	/* tictime is the game time of the command we are creating */

	tictime = latest_player_time[player];
	latest_player_commands[player][tictime % MAX_NET_LAG] = multkey;
	++latest_player_time[player];

	/* Save the current time for lag calculation */

	player_command_time[tictime % MAX_NET_LAG] = Timer_GetMS();

	/* if this is a multiplayer game, send the command */

	if (playmode == PLAYMODE_ASYNCH) {
		asynput(multkey);
	}
}

#if 0
static void dump_cmds(void)
{
	printf("%i: %i, %i\n", countmove,
		latest_player_commands[0][countmove % MAX_NET_LAG],
		latest_player_commands[1][countmove % MAX_NET_LAG]
		);
}
#endif

int swmain(int argc, char *argv[])
{
	int nexttic;

	swinit(argc, argv);
	setjmp(envrestart);

	// sdh 28/10/2001: playmode is called from here now
	// makes for a more coherent progression through the setup process

	if (!playmode) {
		getgamemode();
	}
	swinitlevel();

	nexttic = Timer_GetMS();
	skip_time = 0;

	for (;;) {
		int nowtime;

		/* generate a new move command periodically
		 * and send to other players if necessary */

		nowtime = Timer_GetMS();

		// TODO: Replace the sync code with a PID loop like what
		// Chocolate Doom uses.
		if (nowtime > nexttic
		 && latest_player_time[player] - countmove < MAX_NET_LAG) {

			new_move();

			/* Be accurate (exact amount between tics);
			 * However, if a large spike occurs between tics,
			 * catch up immediately.
			 */

			if (nowtime - nexttic > 1000) {
				nexttic = nowtime + (1000/FPS);
			} else {
				nexttic += (1000 / FPS);
			}

			// wait a bit longer to compensate for lag

			nexttic += skip_time;
			skip_time = 0;
		}

		asynupdate();
		swsndupdate();

		/* if we have all the tic commands we need, we can move */

		if (can_move()) {
			calculate_lag();
			//dump_cmds();
			swmove();
			swdisp();
			swcollsn();
			swsound();
		}

		// sdh 15/11/2001: dont thrash the
		// processor while waiting
		Timer_Sleep(10);
	}

	return 0;
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 25/11/2001: remove intson, intsoff calls
// sdh 15/11/2001: dont thrash the processor while waiting between gametics
// sdh 29/10/2001: harrykeys
// sdh 28/10/2001: conf_ game options
// sdh 28/10/2001: moved auxdisp to swgrpha.c
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent
// sdh 19/10/2001: removed externs, these are now in headers
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 96-12-26        Speed up game a bit
// 87-04-06        Computer plane avoiding oxen.
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 85-10-31        Atari
// 84-06-12        PC-jr Speed-up
// 84-02-02        Development
//
