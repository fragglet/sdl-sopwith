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
//        swend    -      SW end of game
//
//---------------------------------------------------------------------------

#include "bmblib.h"
#include "sw.h"
#include "swcollsn.h"
#include "swend.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swmisc.h"
#include "swsound.h"
#include "swutil.h"

void swend(char *msg, BOOL update)
{
	register char *closmsg = NULL;
	char *multclos(), *asynclos();

	set_type(savemode);
	hires = FALSE;

	sound(0, 0, NULL);
	swsound();

	if (repflag)
		swreport();

	if (playmode == MULTIPLE)
		closmsg = multclos(update);
	else if (playmode == ASYNCH)
		closmsg = asynclos();

	histend();

	puts("\r\n");
	if (closmsg) {
		puts(closmsg);
		puts("\r\n");
	}
	if (msg) {
		puts(msg);
		puts("\r\n");
	}

	inplay = FALSE;
	swflush();
	if (msg || closmsg)
		exit(YES);
	else
		exit(NO);
}





void endgame(int targclr)
{
	register int winclr;
	register OBJECTS *ob;

	if ((playmode != MULTIPLE && playmode != ASYNCH)
	    || multbuff->mu_maxplyr == 1)
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
		if (!endsts[ob->ob_index]) {
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
	register int n = ob->ob_index;

	endsts[n] = WINNER;
	if (n == player) {
		endcount = 72;
		goingsun = TRUE;
		ob->ob_dx = ob->ob_dy = ob->ob_ldx = ob->ob_ldy = 0;
		ob->ob_state = FLYING;
		ob->ob_life = MAXFUEL;
		ob->ob_speed = MIN_SPEED;
	}
}




void loser(OBJECTS * ob)
{
	register int n = ob->ob_index;

	endsts[n] = LOSER;

	if (n == player) {
		swcolour(0x82);
		swposcur(16, 12);
		swputs("THE END");
		endcount = 20;
	}
}




void swreport()
{
	puts("\r\nEnd of game statictics\r\n\r\n");
	puts("Objects used: ");
	dispd(((int) objsmax - (int) objtop + 1) / sizeof(OBJECTS), 0);
	puts("\r\n");
}



//---------------------------------------------------------------------------
//
// $Log: $
//
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

