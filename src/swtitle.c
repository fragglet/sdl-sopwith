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
//
//        swtitle  -      SW perform animation on the title screen
//
//---------------------------------------------------------------------------

#include <ctype.h>

#include "video.h"

#include "sw.h"
#include "swasynio.h"
#include "swconf.h"
#include "swend.h"
#include "swground.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swmisc.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swtitle.h"

#define X_OFFSET ((SCR_WDTH/2)-160)

// sdh -- use network edition title screen

#define NET_TITLE

void swtitln()
{
	int i, h;

	tickmode = 1;

	sound(S_TITLE, 0, NULL);

	// sdh 26/03/2002: remove swinitgrph call

	// clear the screen

	Vid_ClearBuf();

/*---------------- Original BMB Version---------------*/
#ifndef NET_TITLE

	swcolour(3);
	swposcur(13+X_OFFSET/8, 6);
	swputs("S O P W I T H");

	swcolour(1);
	swposcur(12+X_OFFSET/8, 8);
	swputs("(Version " VERSION ")");

	swcolour(3);
	swposcur(5+X_OFFSET/8, 11);
	swputs("(c) Copyright 1984, 1985, 1987");

	swcolour(1);
	swposcur(6+X_OFFSET/8, 12);
	swputs("BMB ");
	swcolour(3);
	swputs("Compuscience Canada Ltd.");
#else
/*------------------ Original BMB Version---------------*/

/*---------------- New Network Version ---------------*/

	swcolour(2);
	swposcur(18+X_OFFSET/8, 2);
	swputs("SDL");

	swcolour(3);
	swposcur(13+X_OFFSET/8, 4);
	swputs("S O P W I T H");

	swposcur(13+X_OFFSET/8, 6);
	swputs("Version " VERSION);

	swcolour(3);
	swposcur(1+X_OFFSET/8, 10);
	swputs("(c) Copyright 1984, 1985, 1987");

	swcolour(1);
	swposcur(5+X_OFFSET/8, 11);
	swputs("BMB ");
	swcolour(3);
	swputs("Compuscience Canada Ltd.");

	swcolour(3);
	swposcur(1+X_OFFSET/8, 12);
	swputs("(c) Copyright 1984-2000 David L. Clark");

	swcolour(3);
	swposcur(1+X_OFFSET/8, 13);
	swputs("(c) Copyright 2001-2003 Simon Howard");

/*---------------- New Network Version-----------------*/

	displx = 700-X_OFFSET;
	dispinit = TRUE;
	swground();

	// sdh 28/06/2002: cleared this up a lot, no more 
	// creating objects etc

	Vid_DispSymbol(260+X_OFFSET, 180, symbol_plane[0][0], 1);
	Vid_DispSymbol(50+X_OFFSET, 180, symbol_plane_win[3], 1);
	Vid_DispSymbol(100+X_OFFSET, ground[800] + 16, symbol_ox[0], 1);
	Vid_DispSymbol(234+X_OFFSET, ground[934] + 16, symbol_targets[3], 2);
	Vid_DispSymbol(20+X_OFFSET, 160, symbol_plane_hit[0], 1);

	for (i = 9, h=150; i; --i, h += 5)
		Vid_PlotPixel(30+X_OFFSET, h, 3);

#endif				/* #ifndef NET_TITLE */
}

void swtitlf()
{

	if (titleflg)
		return;

	sound(0, 0, NULL);
	swsound();
	tickmode = 0;
}


//
// menus
//
// sdh 19/10/2001: moved all the menus into swtitle.c,
// I think they belong here anyway
// rename to swmenus.c?
//

BOOL ctlbreak()
{
	return Vid_GetCtrlBreak();
}

// clear bottom of screen

void clrprmpt()
{
	// sdh: generic clear

	int x, y;

	for (y = 0; y <= 43; ++y)
		for (x = 0; x < SCR_WDTH; ++x) {
			Vid_PlotPixel(x, y, 0);
		}

	swposcur(0, 20);
}

static BOOL gethost()
{
	clrprmpt();

	swputs("Enter Remote Hostname/IP:\n");
	swgets(asynhost, sizeof(asynhost) - 3);

	return 1;
}

// network menu

static BOOL getnet()
{
	for (;;) {
		clrprmpt();
		swputs("Key: L - listen for connection\n");
		swputs("     C - connect to remote host\n");
		swputs("     T - connect to TCP loop\n");

		Vid_Update();

		swsndupdate();

		if (ctlbreak())
			swend(NULL, NO);

		switch (toupper(swgetc() & 0xff)) {
		case 'L':
			asynmode = ASYN_LISTEN;
			return 1;
		case 'C':
			asynmode = ASYN_CONNECT;
			gethost();
			return 1;
		case 'T':
			asynmode = ASYN_TCPLOOP;
			gethost();
			return 1;
		case 27:
			return 0;
		}
	}
}

// controller menu (unused)
#if 0
static void getkey()
{
	register char key;

	/*----------------97/12/27--------------
        clrprmpt();
        swputs( "Key: 1 - Joystick with IBM Keyboard\r\n" );
        swputs( "     2 - Joystick with non-IBM Keyboard\r\n" );
        swputs( "     3 - IBM Keyboard only\r\n" );
        swputs( "     4 - Non-IBM keyboard only\r\n" );
        for (;;) {
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( ( key = swgetc() & 0x00FF ) < '1' )
                        || ( key > '4' ) )
                        continue;
                joystick = ( key <= '2' );
                ibmkeybd = ( key == '1' ) || ( key == '3' );
                return;
        }
        ------------------97/12/27--------------*/
	clrprmpt();
	swputs("Key: K - Keyboard Only\r\n");
	swputs("     J - Joystick and Keyboard\r\n");

	Vid_Update();

	for (;;) {
		swsndupdate();
		if (ctlbreak())
			swend(NULL, NO);
		if (((key = toupper(swgetc() & 0x00FF)) != 'K')
		    && (key != 'J'))
			continue;
		joystick = key == 'J';
		ibmkeybd = 1;
		return;
	}
}
#endif

// game menu for multiplayer (unused)

int getgame()
{
	register int game;

	clrprmpt();
	swputs("         Key a game number");

	for (;;) {
		if (ctlbreak())
			swend(NULL, NO);
		if (((game = (swgetc() & 0x00FF) - '0') >= 0)
		    && (game <= MAX_GAME))
			return (game);
	}
}

// sdh: get single player skill level

static BOOL getskill()
{
	for (;;) {
		clrprmpt();
		swputs("Key: N - novice player\r\n");
		swputs("     E - expert player\r\n");

		Vid_Update();

		swsndupdate();
		if (ctlbreak())
			swend(NULL, NO);
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

void getgamemode()
{
	for (;;) {
		char c;

		swtitln();

		clrprmpt();

		swputs("Key: S - single player\r\n");
		swputs("     C - single player against computer\r\n");
#ifdef TCPIP
		swputs("     N - network game\r\n");
#endif
		swputs("     O - game options\r\n");
		swputs("     Q - quit game\r\n");
		Vid_Update();

		if (ctlbreak())
			swend(NULL, NO);

		c = toupper(swgetc() & 0xff);

		switch (c) {
		case 'S':
			if (getskill())
				return;
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
		case 'Q':
			exit(0);
			break;
		}
	}
}


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.5  2004/03/29 23:58:23  fraggle
// Add a quit option to the main menu
//
// Revision 1.4  2003/06/08 03:41:42  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.3  2003/06/04 15:43:39  fraggle
// Fix year range in copyright
//
// Revision 1.2  2003/04/05 22:55:11  fraggle
// Remove the FOREVER macro and some unused stuff from std.h
//
// Revision 1.1.1.1  2003/02/14 19:03:22  fraggle
// Initial Sourceforge CVS import
//
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
//---------------------------------------------------------------------------

