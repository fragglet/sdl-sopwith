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
//        swend    -      SW end of game
//

#include "sw.h"
#include "swend.h"
#include "swmain.h"
#include "swtext.h"
#include "swsound.h"
#include "swobject.h"

void swend(char *msg, bool update)
{
	char *closmsg = NULL;
	char *multclos(), *asynclos();

	sound(0, 0, NULL);
	swsound();

	if (playmode == PLAYMODE_ASYNCH) {
		closmsg = asynclos();
	}

	puts("\n");
	if (closmsg) {
		puts(closmsg);
		puts("\n");
	}
	if (msg) {
		puts(msg);
		puts("\n");
	}

	if (msg || closmsg) {
		exit(1);
	} else {
		exit(0);
	}
}





void endgame(int targclr)
{
	int winclr;
	OBJECTS *ob;

	if (playmode != PLAYMODE_ASYNCH) {
		winclr = 1;
	} else if ((objtop + 1)->ob_score.score == objtop->ob_score.score) {
		winclr = 3 - targclr;
	} else {
		winclr = ((objtop + 1)->ob_score.score
		       > objtop->ob_score.score) + 1;
	}

	ob = objtop;
	while (ob->ob_type == PLANE) {
		if (ob->ob_endsts == PLAYING) {
			if (ob->ob_clr == winclr
			 && (ob->ob_crashcnt < (MAXCRASH - 1)
			  || (ob->ob_crashcnt < MAXCRASH
			   && !plane_is_killed(ob->ob_state)))) {
				winner(ob);
			} else {
				loser(ob);
			}
		}
		ob = ob->ob_next;
	}
}



void winner(OBJECTS *ob)
{
	ob->ob_endsts = WINNER;
	ob->ob_goingsun = true;
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

	if (ob == consoleplayer) {
		endcount = 20;
	}
}

void dispendmessage(void)
{
	if (consoleplayer->ob_endsts != PLAYING) {
		swcolor(0x82);
		swposcur((SCR_WDTH/16) - 4, 12);
		swputs("THE END");
	}
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
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
