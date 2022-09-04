// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
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
//
//        swasynio -      SW asynchrounous communications I/O
//
//---------------------------------------------------------------------------

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

asynmode_t asynmode;
char asynhost[128];

#define SYNC_IM_PLAYER0 '?'
#define SYNC_IM_PLAYER1 '!'
#define TIMEOUT_LEN_MS 5000 	/* time out after 5 seconds */

static int timeout_time;

static void settimeout(void)
{
	timeout_time = Timer_GetMS() + TIMEOUT_LEN_MS;
}

static BOOL timeout(void)
{
	return Timer_GetMS() >= timeout_time;
}

static inline void sendshort(int s)
{
	commout(s & 0xff);
	commout((s >> 8) & 0xff);
}

static inline int try_readshort(void)
{
	int s, t;

	s = commin();

	if (s < 0) {
		return -1;
	}

	settimeout();

	while ((t = commin()) < 0) {
		if (timeout()) {
			fprintf(stderr, "readshort: timeout on read\n");
			exit(-1);
		}
	}

	return (t << 8) + s;
}

static int readshort(void)
{
	int i;

	settimeout();

	for (i=-1; i < 0; i = try_readshort()) {
		if (timeout()) {
			fprintf(stderr, "readshort: timeout on read\n");
			exit(-1);
		}
	}

	return i;
}

void asynput(int movekey)
{
	sendshort(movekey);
}

char *asynclos(void)
{
	commterm();
	return NULL;
}


void asynupdate(void)
{
	int i, ticnum;

	i = try_readshort();

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

static void synchronize(void)
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
	settimeout();

	for (p = buf; *p;) {
		int c;

		if (timeout()) {
			fprintf(stderr, "asyninit: timeout on connect\n");
			exit(-1);
		}

		c = commin();

		if (c >= 0) {
			if (c == *p) {
				++p;
			} else if (c != SYNC_IM_PLAYER0
			        && c != SYNC_IM_PLAYER1) {
				fprintf(stderr, "asyninit: invalid protocol"
				        " header received!\n");
				exit(-1);
			}
		}
	}

	free(buf);

	if (player) {
		explseed = readshort();

		printf("random seed: %i\n", explseed);
		conf_missiles = readshort() != 0;
		conf_wounded = readshort() != 0;
		conf_animals = readshort() != 0;
		conf_big_explosions = readshort() != 0;
	} else {
		// send settings
		sendshort(explseed);

		printf("random seed: %i\n", explseed);
		sendshort(conf_missiles);
		sendshort(conf_wounded);
		sendshort(conf_animals);
		sendshort(conf_big_explosions);
	}
}

// setup tcp loop
static void AssignPlayers(BOOL server_side)
{
	int starttime, lastsendtime, now;

	clrprmpt();
	swputs("  Connected, waiting for other player\n");
	Vid_Update();

	starttime = Timer_GetMS();
	lastsendtime = 0;

	// for the first 5 seconds, listen to see if theres another player
	// there
	for (;;) {
		int c;

		if (ctlbreak()) {
			fprintf(stderr, "asyninit: user aborted\n");
			exit(-1);
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
				fprintf(stderr,
				        "asyninit: got wrong char: %d\n", c);
				exit(-1);
			}
		}

		now = Timer_GetMS();
		if (!server_side) {
			// After five seconds we switch to server mode, so
			// that two clients connected to a loop synchronize:
			if ((now - starttime) > 5000) {
				server_side = TRUE;
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
		swputs("  Listening for connection...");
		Vid_Update();
		commlisten();
		AssignPlayers(TRUE);
	} else if (asynmode == ASYN_CONNECT) {
		swtitln();
		clrprmpt();
		swputs("  Attempting to connect to \n  ");
		swputs(asynhost);
		swputs(" ...");
		Vid_Update();
		commconnect(asynhost);
		AssignPlayers(FALSE);
	} else {
		fprintf(stderr, "unknown asynmode mode\n");
		exit(-1);
	}
}

void init1asy(void)
{
#ifndef TCPIP
	fprintf(stderr, "TCP/IP support not compiled into binary!\n");
	return;
#else
	asyninit();
	clrprmpt();
	swputs("        Waiting for other player");
	synchronize();
#endif
}



void init2asy(void)
{
	initplyr(NULL);
	initplyr(NULL);
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.10  2005/05/29 19:46:10  fraggle
// Fix up autotools build. Fix "make dist".
//
// Revision 1.9  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.8  2004/10/20 18:17:06  fraggle
// Remove dead code
//
// Revision 1.7  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.6  2004/10/15 17:52:31  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.5  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.4.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.4  2003/04/05 22:55:11  fraggle
// Remove the FOREVER macro and some unused stuff from std.h
//
// Revision 1.3  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.2  2003/04/05 22:31:29  fraggle
// Remove PLAYMODE_MULTIPLE and swnetio.c
//
// Revision 1.1.1.1  2003/02/14 19:03:08  fraggle
// Initial Sourceforge CVS import
//
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
//---------------------------------------------------------------------------

