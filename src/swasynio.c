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
//
//        swasynio -      SW asynchrounous communications I/O
//
//---------------------------------------------------------------------------

#include <ctype.h>

#include "tcpcomm.h"
#include "cgavideo.h"
#include "timer.h"

#include "sw.h"
#include "swasynio.h"
#include "swdisp.h"
#include "swend.h"
#include "swgames.h"
#include "swinit.h"
#include "swmain.h"
#include "swmisc.h"
#include "swnetio.h"
#include "swsound.h"
#include "swtitle.h"

asynmode_t asynmode;
char asynhost[128];

static int lastkey = 0;		/*  Always behind one character     */
static int timeout_time;

static void settimeout(int ms)
{
	timeout_time = Timer_GetMS() + ms;
}

static BOOL timeout()
{
	return Timer_GetMS() >= timeout_time;
}

static inline void sendshort(int s)
{
	commout(s & 0xff);
	commout((s >> 8) & 0xff);
}

static inline int readshort()
{
	int s, t;

	while ((s = commin()) < 0) {
		if (timeout()) {
			fprintf(stderr, "readshort: timeout on read\n");
			exit(-1);
		}
	}

	while ((t = commin()) < 0) {
		if (timeout()) {
			fprintf(stderr, "readshort: timeout on read\n");
			exit(-1);
		}
	}

	return (t << 8) + s;
}

static int asynin()
{
	register int c;

	settimeout(500);
	FOREVER {
		if ((c = commin()) >= 0)
			return c;
		if (ctlbreak())
			swend(NULL, NO);
		if (timeout())
			return -1;
	}
}


int asynget(OBJECTS * ob)
{
	register int key;

	if (ob->ob_index == player) {
		key = lastkey;
		lastkey = 0;
	} else {
		settimeout(1000);
		key = readshort();
	}

	return key;
}

void asynput()
{
	static BOOL first = TRUE;

	if (first)
		first = FALSE;
	else {
		lastkey = CGA_GetGameKeys();
		
		if (conf_harrykeys && nobjects[player].ob_orient)
			lastkey ^= K_FLAPU | K_FLAPD;
	}

	swflush();

	sendshort(lastkey);
}

char *asynclos(BOOL update)
{
	commterm();
	return NULL;
}

#define PROTOHEADER "SDLSOPWITH" VERSION

static void synchronize()
{
	// check for header

	// send the header ourselves first

	char *buf;
	char *p;

	buf = malloc(sizeof(PROTOHEADER) + 5);
	sprintf(buf, PROTOHEADER "%i", player);

	for (p = buf; *p; ++p)
		commout(*p);

	// now listen for response

	sprintf(buf, PROTOHEADER "%i", !player);

	settimeout(2000);

	for (p = buf; *p;) {
		int c;

		if (timeout()) {
			fprintf(stderr,
				"synchronize: timeout on connect\n");
			exit(-1);
		}

		c = commin();

		if (c >= 0) {
			if (c == *p) {
				++p;
			} else {
				fprintf(stderr,
					"synchronize: invalid protocol header received!\n");
				exit(-1);
			}
		}
	}

	free(buf);

	if (player) {
		settimeout(1000);
		explseed = readshort();
		settimeout(1000);
		conf_missiles = readshort() != 0;
		conf_wounded = readshort() != 0;
		conf_animals = readshort() != 0;
	} else {
		// send settings
		sendshort(explseed);
		sendshort(conf_missiles);
		sendshort(conf_wounded);
		sendshort(conf_animals);
	}
}

// setup tcp loop

static void tcploop_connect()
{
	int time;

	clrprmpt();
	swputs("  Attempting to connect to\n  ");
	swputs(asynhost);
	swputs(" ...");
	CGA_Update();
	
	commconnect(asynhost);

	clrprmpt();
	swputs("  Connected, waiting for other player\n");
	CGA_Update();

	// for the first 5 seconds, listen to see if theres another player 
	// there

	for (time = Timer_GetMS() + 5000; Timer_GetMS() < time; ) {
		int c;

		if (ctlbreak()) {
			fprintf(stderr, "tcploop_connect: user aborted\n");
			exit(-1);
		}
		
		c = commin();

		if (c >= 0) {
			if (c == '?') {
				commout('!');
				player = 1;
				return;
			} else {
				fprintf(stderr, 
					"tcploop_connect: invalid"
					"char recieved\n");
				exit(-1);
			}
		}
	}

	// now send a ? every second until we get a ! response

	time = Timer_GetMS() + 1000;

	FOREVER {
		int c;

		if (ctlbreak()) {
			fprintf(stderr, "tcploop_connect: user aborted\n");
			exit(-1);
		}

		c = commin();

		if (c >= 0) {
			if (c == '!') {
				player = 0;
				return;
			} else {
				fprintf(stderr, 
					"tcploop_connect: invalid"
					"char recieved\n");
				exit(-1);
			}
		}
	

		if (Timer_GetMS() > time) {
			commout('?');
			time = Timer_GetMS() + 1000;
		}
	}
}

// setup connection

void asyninit()
{
	if (asynmode == ASYN_TCPLOOP) {
		tcploop_connect();
	} else if (asynmode == ASYN_LISTEN) {
		clrprmpt();
		swputs("  Listening for connection...");
		CGA_Update();
		commlisten();
		player = 0;
	} else if (asynmode == ASYN_CONNECT) {
		clrprmpt();
		swputs("  Attempting to connect to \n  ");
		swputs(asynhost);
		swputs(" ...");
		CGA_Update();
		commconnect(asynhost);
		player = 1;
	} else {
		fprintf(stderr, "unknown asynmode mode\n");
		exit(-1);
	}
}

void init1asy()
{
#ifndef TCPIP
	fprintf(stderr, "TCP/IP support not compiled into binary!\n");
	return;
#else
	asyninit();
	clrprmpt();
	swputs("        Waiting for other player");
	synchronize();

	currgame = &swgames[0];
	multbuff->mu_numplyr = multbuff->mu_maxplyr = 2;
#endif
}



void init2asy()
{
	register OBJECTS *ob;
	OBJECTS *initpln();

	if (!player)
		initplyr(NULL);

	ob = initpln(NULL);
	ob->ob_drawf = dispmult;
	ob->ob_movef = movemult;
	ob->ob_clr = ob->ob_index % 2 + 1;
	ob->ob_owner = ob;
	ob->ob_state = FLYING;
	oobjects[ob->ob_index] = *ob;
//	movmem(ob, &oobjects[ob->ob_index], sizeof(OBJECTS));

	if (player)
		initplyr(NULL);

	commout(0);
	commout(0);
}

//---------------------------------------------------------------------------
//
// $Log: $
//
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

