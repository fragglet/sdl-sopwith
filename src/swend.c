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
//        swend    -      SW end of game
//
//---------------------------------------------------------------------------

#include "sw.h"
#include "swcollsn.h"
#include "swend.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swtext.h"
#include "swsound.h"
#include "swutil.h"


static void swreport()
{
	puts("\nEnd of game statistics\n");
	puts("Objects used: ");
	printf("%d\n", ((int) objsmax - (int) objtop + 1) / sizeof(OBJECTS));
	puts("\n");
}

void swend(char *msg, BOOL update)
{
	register char *closmsg = NULL;
	char *multclos(), *asynclos();

	sound(0, 0, NULL);
	swsound();

	if (repflag)
		swreport();

        if (playmode == PLAYMODE_ASYNCH)
		closmsg = asynclos();

	histend();

	puts("\n");
	if (closmsg) {
		puts(closmsg);
		puts("\n");
	}
	if (msg) {
		puts(msg);
		puts("\n");
	}

	inplay = FALSE;

	if (msg || closmsg)
		exit(YES);
	else
		exit(NO);
}





void endgame(int targclr)
{
	register int winclr;
	register OBJECTS *ob;

	if (playmode != PLAYMODE_ASYNCH)
		winclr = 1;
	else {
		if ((objtop + 1)->ob_score == objtop->ob_score)
			winclr = 3 - targclr;
		else
			winclr =
			    ((objtop + 1)->ob_score >
			     objtop->ob_score) + 1;
	}

	ob = objtop;
	while (ob->ob_type == PLANE) {
		if (ob->ob_endsts == PLAYING) {
			if (ob->ob_clr == winclr
			    && (ob->ob_crashcnt < (MAXCRASH - 1)
				|| (ob->ob_crashcnt < MAXCRASH
				    && (ob->ob_state == FLYING
					|| ob->ob_state == STALLED
					|| ob->ob_state == WOUNDED
					|| ob->ob_state == WOUNDSTALL))))
				winner(ob);
			else
				loser(ob);
		}
		ob = ob->ob_next;
	}
}



void winner(OBJECTS * obp)
{
	register OBJECTS *ob = obp;

	ob->ob_endsts = WINNER;
	ob->ob_goingsun = TRUE;
	ob->ob_dx = ob->ob_dy = ob->ob_ldx = ob->ob_ldy = 0;
	ob->ob_state = FLYING;
	ob->ob_life = MAXFUEL;
	ob->ob_speed = MIN_SPEED;

	if (ob == consoleplayer) {
		endcount = 72;
	}
}


void loser(OBJECTS * ob)
{
	ob->ob_endsts = LOSER;

	// sdh 28/4/2002: change swposcur to center screen on 
	// non-320 pixel wide screens

	if (ob == consoleplayer) {
		endcount = 20;
	}
}

void dispendmessage()
{
	if (consoleplayer->ob_endsts != PLAYING) {
		swcolour(0x82);
		swposcur((SCR_WDTH/16) - 4, 12);
		swputs("THE END");
	}
}



//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.7  2004/10/25 19:58:06  fraggle
// Remove 'goingsun' global variable
//
// Revision 1.6  2004/10/20 19:00:01  fraggle
// Remove currobx, endsts variables
//
// Revision 1.5  2004/10/15 17:52:32  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.4  2003/06/08 02:39:25  fraggle
// Initial code to remove XOR based drawing
//
// Revision 1.3  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.2  2003/04/05 22:31:29  fraggle
// Remove PLAYMODE_MULTIPLE and swnetio.c
//
// Revision 1.1.1.1  2003/02/14 19:03:10  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed externs, these are now in headers
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 84-02-02        Development
//
//---------------------------------------------------------------------------

