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
//        swtitle  -      SW perform animation on the title screen
//
//---------------------------------------------------------------------------

#include <ctype.h>

#include "cgavideo.h"

#include "sw.h"
#include "swasynio.h"
#include "swconf.h"
#include "swend.h"
#include "swground.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swmisc.h"
#include "swplanes.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swtitle.h"

// sdh -- use network edition title screen

#define NET_TITLE

void swtitln()
{
	OBJECTS ob;
	register int i, h;

	tickmode = 1;

	sound(S_TITLE, 0, NULL);

	// if solid_ground has been changed this needs to be called
	// again
	
	swinitgrph(); 

	// clear the screen

	setvdisp();
	clrdispv();

/*---------------- Original BMB Version---------------*/
#ifndef NET_TITLE

	swcolour(3);
	swposcur(13, 6);
	swputs("S O P W I T H");

	swcolour(1);
	swposcur(12, 8);
	swputs("(Version " VERSION ")");

	swcolour(3);
	swposcur(5, 11);
	swputs("(c) Copyright 1984, 1985, 1987");

	swcolour(1);
	swposcur(6, 12);
	swputs("BMB ");
	swcolour(3);
	swputs("Compuscience Canada Ltd.");
#else
/*------------------ Original BMB Version---------------*/

/*---------------- New Network Version ---------------*/

	swcolour(2);
	swposcur(18, 2);
	swputs("SDL");

	swcolour(3);
	swposcur(13, 4);
	swputs("S O P W I T H");

	swposcur(13, 6);
	swputs("Version " VERSION);

	swcolour(3);
	swposcur(1, 10);
	swputs("(c) Copyright 1984, 1985, 1987");

	swcolour(1);
	swposcur(5, 11);
	swputs("BMB ");
	swcolour(3);
	swputs("Compuscience Canada Ltd.");

	swcolour(3);
	swposcur(1, 12);
	swputs("(c) Copyright 1984-2000 David L. Clark");

	swcolour(3);
	swposcur(1, 13);
	swputs("(c) Copyright 2001 Simon Howard");

/*---------------- New Network Version-----------------*/

	displx = 700;
	dispinit = TRUE;
	swground();

	ob.ob_type = PLANE;
	ob.ob_symhgt = ob.ob_symwdt = 16;
	ob.ob_clr = 1;
	ob.ob_newsym = swplnsym[0][0];
	swputsym(260, 180, &ob);

	ob.ob_newsym = swwinsym[3];
	swputsym(50, 180, &ob);

	ob.ob_type = OX;
	ob.ob_newsym = swoxsym[0];
	swputsym(100, ground[800] + 16, &ob);

	ob.ob_type = TARGET;
	ob.ob_clr = 2;
	ob.ob_newsym = swtrgsym[3];
	swputsym(234, ground[934] + 16, &ob);

	ob.ob_type = PLANE;
	ob.ob_newsym = swhitsym[0];
	swputsym(20, 160, &ob);

	ob.ob_type = SMOKE;
	ob.ob_symhgt = ob.ob_symwdt = 1;
	ob.ob_newsym = (char *) 0x82;
	h = 150;
	for (i = 9; i; --i)
		swputsym(30, h += 5, &ob);

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
	return CGA_GetCtrlBreak();;
}

// clear bottom of screen

void clrprmpt()
{
	// sdh: generic clear

	int x, y;

	for (y = 0; y <= 43; ++y)
		for (x = 0; x < SCR_WDTH; ++x) {
			swpntsym(x, y, 0);
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
	FOREVER {
		clrprmpt();
		swputs("Key: L - listen for connection\n");
		swputs("     C - connect to remote host\n");

		CGA_Update();

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
        FOREVER {
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

	CGA_Update();

	FOREVER {
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
	FOREVER {
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
	FOREVER {
		clrprmpt();
		swputs("Key: N - novice player\r\n");
		swputs("     E - expert player\r\n");

		CGA_Update();

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

void getmode()
{
	FOREVER {
		char c;

		swtitln();

		clrprmpt();
		swputs("Key: S - single player\r\n");
		swputs("     C - single player against computer\r\n");
		swputs("     N - network game\r\n");
		swputs("     O - game options\r\n");
		CGA_Update();

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
		case 'N':
			if (getnet()) {
				playmode = PLAYMODE_ASYNCH;
				return;
			}
			break;
		default:
			break;
		}
	}
}


//---------------------------------------------------------------------------
//
// $Log: $
//
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

