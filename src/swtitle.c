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
//        swtitle  -      SW perform animation on the title screen
//

#include <ctype.h>
#include <string.h>

#include "video.h"

#include "sw.h"
#include "swasynio.h"
#include "swconf.h"
#include "swend.h"
#include "swgames.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swtext.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swtitle.h"

#define X_OFFSET ((SCR_WDTH/2)-160)

void swtitln(void)
{
	int i, h;

	sound(S_TITLE, 0, NULL);

	// clear the screen
	Vid_ClearBuf();

	swcolor(2);
	swposcur(18+X_OFFSET/8, 2);
	swputs("SDL");

	swcolor(3);
	swposcur(13+X_OFFSET/8, 4);
	swputs("S O P W I T H");

	swposcur(13+X_OFFSET/8, 6);
	swputs("Version " PACKAGE_VERSION);

	swcolor(3);
	swposcur(0+X_OFFSET/8, 9);
	swputs("(c) 1984, 1985, 1987 ");

	swcolor(1);
	swputs("BMB ");
	swcolor(3);
	swputs("Compuscience");

	swcolor(3);
	swposcur(0+X_OFFSET/8, 10);
	swputs("(c) 1984-2000 David L. Clark");

	swcolor(3);
	swposcur(0+X_OFFSET/8, 11);
	swputs("(c) 2001-2023 Simon Howard, Jesse Smith");

	swcolor(3);
	swposcur(0+X_OFFSET/8, 12);
	swputs("    Distributed under the ");
	swcolor(1);
	swputs("GNU");
	swcolor(3);
	swputs(" GPL");

	// We might be playing a custom level, but we need to swap back to
	// the original level to render the title screen properly.
	currgame = &original_level;
	initgrnd();
	displx = 507-X_OFFSET;
	swground();

	Vid_DispSymbol(40+X_OFFSET, 180, symbol_plane[0][0], OWNER_PLAYER1);
	Vid_DispSymbol(130+X_OFFSET, 80, symbol_plane[1][7], OWNER_PLAYER2);
	Vid_DispSymbol(23+X_OFFSET, ground[530] + 16, symbol_targets[3],
	               OWNER_PLAYER2);
	Vid_DispSymbol(213+X_OFFSET, ground[720] + 16, symbol_ox[0],
	               OWNER_PLAYER1);
	Vid_DispSymbol(270+X_OFFSET, 160, symbol_plane_hit[0],
	               OWNER_PLAYER2);

	for (i = 6, h=165; i; --i, h += 5) {
		Vid_PlotPixel(280+X_OFFSET, h, 3);
	}
}

void swtitlf(void)
{
	if (titleflg) {
		return;
	}

	sound(0, 0, NULL);
	swsound();
}


//
// menus
//

bool ctlbreak(void)
{
	return Vid_GetCtrlBreak();
}

// clear bottom of screen

void clrprmpt(void)
{
	int x, y;

	for (y = 0; y <= 43; ++y)
		for (x = 0; x < SCR_WDTH; ++x) {
			Vid_PlotPixel(x, y, 0);
		}

	swposcur(0, 20);
}

static bool gethost(void)
{
	clrprmpt();

	swputs("Enter Remote Hostname/IP:\n");
	swgets(asynhost, sizeof(asynhost) - 3);

	return strcmp(asynhost, "") != 0;
}

// network menu

static bool getnet(void)
{
	for (;;) {
		clrprmpt();
		swputs("Key: L - listen for connection\n");
		swputs("     C - connect to remote host\n");

		Vid_Update();

		swsndupdate();

		if (ctlbreak()) {
			swend(NULL, false);
		}

		switch (toupper(swgetc() & 0xff)) {
		case 'L':
			asynmode = ASYN_LISTEN;
			return 1;
		case 'C':
			asynmode = ASYN_CONNECT;
			return gethost();
		case 27:
			return 0;
		}
	}
}

// sdh: get single player skill level

static bool getskill(void)
{
	for (;;) {
		clrprmpt();
		swputs("Key: N - novice player\n");
		swputs("     E - expert player\n");

		Vid_Update();

		swsndupdate();
		if (ctlbreak()) {
			swend(NULL, false);
		}
		switch (toupper(swgetc() & 0xff)) {
		case 'N':
			playmode = PLAYMODE_NOVICE;
			return 1;
		case 'E':
			playmode = PLAYMODE_SINGLE;
			return 1;
		case 27:
			return 0;
		}
	}
}

void getgamemode(void)
{
	for (;;) {
		char c;

		swtitln();

		clrprmpt();

		swputs("Key: S - single player\n");
		swputs("     C - single player against computer\n");
#ifdef TCPIP
		swputs("     N - network game\n");
#endif
		swputs("     O - game options\n");
#ifndef NO_EXIT
		swputs("     Q - quit game\n");
#endif
		Vid_Update();

		if (ctlbreak()) {
			swend(NULL, false);
		}

		c = toupper(swgetc() & 0xff);

		switch (c) {
		case 'S':
			if (getskill()) {
				return;
			}
			break;
		case 'O':
			setconfig();
			break;
		case 'C':
			playmode = PLAYMODE_COMPUTER;
			return;
#ifdef TCPIP
		case 'N':
			if (getnet()) {
				playmode = PLAYMODE_ASYNCH;
				return;
			}
			break;
#endif
#ifndef NO_EXIT
		case 'Q':
			exit(0);
			break;
#endif
		}
	}
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/06/2002: move to new sopsym_t for symbols,
//                 remove references to symwdt, symhgt
// sdh 28/04/2002: Centering of title screen on non-320 pixel wide screens
// sdh 26/03/2002: change CGA_ to Vid_
// sdh 16/11/2001: TCPIP #define to disable TCP/IP support
// sdh 29/10/2001: moved options menu into swconf.c
// sdh 29/10/2001: harrykeys
// sdh 28/10/2001: rearranged title/menu stuff a bit
//                 added options menu
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 19/10/2001: moved all the menus here
// sdh 19/10/2001: removed extern definitions, these are now in headers
// sdh 18/10/2001: converted all functions to ANSI-style arguments
// sdh ??/10/2001: added #define to control whether we use the classic
//                 title screen or the "network edition" one
//
// 2000-10-29      Copyright update.
// 99-01-24        1999 copyright.
// 96-12-27        New network version.
// 87-04-01        Version 7.F15
// 87-03-11        Title reformatting.
// 87-03-10        Microsoft compiler.
// 84-02-02        Development
//
