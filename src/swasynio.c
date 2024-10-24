//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// You can redistribute and/or modify this program under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation, or any later version. This program is distributed WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
//
//
//        swasynio -      SW asynchrounous communications I/O
//

#include "tcpcomm.h"
#include "timer.h"
#include "video.h"

#include "sw.h"
#include "swasynio.h"
#include "swgames.h"
#include "swinit.h"
#include "swmain.h"
#include "swtext.h"
#include "swtitle.h"

#define ABORTMSG "\n\n         (Ctrl-C to abort)"

int asynport = DEFAULT_PORT;
enum asyn_mode asynmode;
char asynhost[128];

#define SYNC_IM_PLAYER0 '?'
#define SYNC_IM_PLAYER1 '!'
#define TIMEOUT_LEN_MS 5000 	/* time out after 5 seconds */

static int timeout_time;

static void SetTimeout(void)
{
	timeout_time = Timer_GetMS() + TIMEOUT_LEN_MS;
}

static bool TimedOut(void)
{
	return Timer_GetMS() >= timeout_time;
}

static inline void SendShort(int s)
{
	commout(s & 0xff);
	commout((s >> 8) & 0xff);
}

static inline int TryReadShort(void)
{
	int s, t;

	s = commin();

	if (s < 0) {
		return -1;
	}

	SetTimeout();

	while ((t = commin()) < 0) {
		if (TimedOut()) {
			error_exit("ReadShort: timeout on read");
		}
	}

	return (t << 8) + s;
}

static int ReadShort(void)
{
	int i;

	SetTimeout();

	for (i=-1; i < 0; i = TryReadShort()) {
		if (TimedOut()) {
			error_exit("ReadShort: timeout on read");
		}
	}

	return i;
}

void asynput(int movekey)
{
	SendShort(movekey);
}

char *asynclos(void)
{
	commterm();
	return NULL;
}

void asynupdate(void)
{
	int i, ticnum;

	i = TryReadShort();

	if (i >= 0) {
		int netplayer;

		/* we have read a short from the other player. add
		 * to the queue */

		netplayer = !player;

		ticnum = latest_player_time[netplayer] % MAX_NET_LAG;
		latest_player_commands[netplayer][ticnum] = i;
		++latest_player_time[netplayer];
	}
}

#define PROTOHEADER_FMT (PACKAGE_STRING ", player %d")

static void Synchronize(void)
{
	char *buf, *p;

	// send the header ourselves first
	buf = malloc(sizeof(PROTOHEADER_FMT) + 5);
	snprintf(buf, sizeof(PROTOHEADER_FMT) + 5, PROTOHEADER_FMT, player);

	for (p = buf; *p; ++p) {
		commout(*p);
	}

	// now listen for response
	snprintf(buf, sizeof(PROTOHEADER_FMT) + 5, PROTOHEADER_FMT, !player);
	SetTimeout();

	for (p = buf; *p;) {
		int c;

		if (TimedOut()) {
			error_exit("Synchronize: timeout on connect");
		}

		c = commin();

		if (c >= 0) {
			if (c == *p) {
				++p;
			} else if (c != SYNC_IM_PLAYER0
			        && c != SYNC_IM_PLAYER1) {
				error_exit("Synchronize: invalid protocol "
				           "header received.");
			}
		}
	}

	free(buf);

	if (player) {
		explseed = ReadShort();

		printf("random seed: %i\n", explseed);
		conf_missiles = ReadShort() != 0;
		conf_wounded = ReadShort() != 0;
		conf_animals = ReadShort() != 0;
		conf_big_explosions = ReadShort() != 0;
		starting_level = ReadShort();
	} else {
		// send settings
		SendShort(explseed);

		printf("random seed: %i\n", explseed);
		SendShort(conf_missiles);
		SendShort(conf_wounded);
		SendShort(conf_animals);
		SendShort(conf_big_explosions);
		SendShort(starting_level);
	}
	gamenum = starting_level;
}

// setup tcp loop
static void AssignPlayers(bool server_side)
{
	int starttime, lastsendtime, now;

	clrprmpt();
	swputs("  Connected, waiting for other player..." ABORTMSG);
	Vid_Update();

	starttime = Timer_GetMS();
	lastsendtime = 0;

	// for the first 5 seconds, listen to see if theres another player
	// there
	for (;;) {
		int c;

		if (ctlbreak()) {
			error_exit("asyninit: user aborted");
		}
		
		c = commin();
		if (c >= 0) {
			if (server_side && c == SYNC_IM_PLAYER0) {
				commout(SYNC_IM_PLAYER1);
				player = 1;
				return;
			} else if (!server_side && c == SYNC_IM_PLAYER1) {
				player = 0;
				return;
			} else if (server_side || c != SYNC_IM_PLAYER0) {
				error_exit("asyninit: got wrong char: %02x",
				           c);
			}
		}

		now = Timer_GetMS();
		if (!server_side) {
			// After five seconds we switch to server mode, so
			// that two clients connected to a loop synchronize:
			if ((now - starttime) > 5000) {
				server_side = true;
			} else if ((now - lastsendtime) > 500) {
				commout(SYNC_IM_PLAYER0);
				lastsendtime = now;
			}
		}
	}
}

// setup connection

static void asyninit(void)
{
	if (asynmode == ASYN_LISTEN) {
		swtitln();
		clrprmpt();
		swputs("  Listening for connection..." ABORTMSG);
		Vid_Update();
		commlisten();
		AssignPlayers(true);
	} else if (asynmode == ASYN_CONNECT) {
		swtitln();
		clrprmpt();
		swputs("  Attempting to connect to\n  ");
		swputs(asynhost);
		swputs(" ...");
		Vid_Update();
		commconnect(asynhost);
		AssignPlayers(false);
	} else {
		error_exit("asynmode: unknown asynmode %d", asynmode);
	}
}

void init1asy(void)
{
#ifndef TCPIP
	error_exit("TCP/IP support not compiled into binary!");
#else
	asyninit();
	clrprmpt();
	swputs("  Waiting for other player...");
	Synchronize();
#endif
}



void init2asy(void)
{
	initplyr(NULL);
	initplyr(NULL);
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 26/03/2002: changed CGA_ to Vid_
// sdh 16/11/2001: TCPIP #define to disable TCP/IP support
// sdh 29/10/2001: harrykeys
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 20/10/2001: rearranged netgame syncronisation, take game settings
//                 from the first (listening) player
// sdh 19/10/2001: removed all externs, this is done with headers now
// sdh 18/10/2001: converted all functions to ansi-style arguments
//
// 96-12-26        Remove IMAGINET network card address in "multaddr".
// 87-03-12        Allow asynch loopback for debugging.
// 87-03-09        Microsoft compiler.
// 85-04-03        Development
//

