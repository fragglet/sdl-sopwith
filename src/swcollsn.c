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
//        swcollsn -      SW collision resolution
//
//---------------------------------------------------------------------------

#include "video.h"

#include "sw.h"
#include "swcollsn.h"
#include "swdisp.h"
#include "swend.h"
#include "swground.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swtext.h"
#include "swmove.h"
#include "swsound.h"
#include "swsplat.h"

static OBJECTS *killed[MAX_OBJS*2], *killer[MAX_OBJS*2];
static int killptr;

static int collsdx[MAX_PLYR];
static int collsdy[MAX_PLYR];
static OBJECTS *collsno[MAX_PLYR];
static int collptr;
static int collxadj, collyadj;


// sdh 28/6/2002: new collision detection code done in memory rather
//                than using the drawing functions
// sdh 27/7/2002: removed old collision detection code

//#define COLL_DEBUG

static void colltest(OBJECTS * ob1, OBJECTS * ob2)
{
	int x, y;
	int x1, y1, x2, y2;
	int w, h;
	unsigned char *data1, *data2;

	if ((ob1->ob_type == PLANE && ob1->ob_state >= FINISHED)
	    || (ob2->ob_type == PLANE && ob2->ob_state >= FINISHED)
	    || (ob1->ob_type == EXPLOSION && ob2->ob_type == EXPLOSION))
		return;

	// (x1, y1) are the coords of the area we are testing in ob1
	// (x2, y2) are the coords of the area in ob2
	// (w, h) is the size of the area

	// x:

	if (ob1->ob_x < ob2->ob_x) {
		x1 = ob2->ob_x - ob1->ob_x;
		x2 = 0;
		w = ob1->ob_newsym->w - x1;
		if (w > ob2->ob_newsym->w)
			w = ob2->ob_newsym->w;
	} else {
		x1 = 0;
		x2 = ob1->ob_x - ob2->ob_x;
		w = ob2->ob_newsym->w - x2;
		if (w > ob1->ob_newsym->w)
			w = ob1->ob_newsym->w;
	}

	// no intersection?

	if (w <= 0)     
		return;

	// y:

	if (ob1->ob_y < ob2->ob_y) {
		y1 = 0;
		y2 = ob2->ob_y - ob1->ob_y;
		h = ob2->ob_newsym->h - y2;
		if (h > ob1->ob_newsym->h)
			h = ob1->ob_newsym->h;
	} else {
		y1 = ob1->ob_y - ob2->ob_y;
		y2 = 0;
		h = ob1->ob_newsym->h - y1;
		if (h > ob2->ob_newsym->h)
			h = ob2->ob_newsym->h;
	}

	// no intersection?

	if (h <= 0)
		return;

#ifdef COLL_DEBUG
	fprintf(stderr,
		"collision test: (%i, %i) at (%i, %i)/(%i, %i)\n",
		w, h, x1, y1, x2, y2);

	fprintf(stderr,
		"info: (%i, %i)/(%i, %i)  (%i, %i)/(%i, %i)\n",
		ob1->ob_x, ob1->ob_y, ob1->ob_newsym->w, ob1->ob_newsym->h,
		ob2->ob_x, ob2->ob_y, ob2->ob_newsym->w, ob2->ob_newsym->h);
#endif

	data1 = ob1->ob_newsym->data + ob1->ob_newsym->w * y1 + x1;
	data2 = ob2->ob_newsym->data + ob2->ob_newsym->w * y2 + x2;

	for (y=0; y<h; ++y) {
		unsigned char *d1 = data1, *d2 = data2;

		for (x=0; x<w; ++x) {
			if (*d1 && *d2) {

				// a collision

				if (killptr < 2*MAX_OBJS - 1) {
					killed[killptr] = ob1;
					killer[killptr] = ob2;
					++killptr;
					killed[killptr] = ob2;
					killer[killptr] = ob1;
					++killptr;
				}
				return; 
			}

			++d1; ++d2;
		}

		data1 += ob1->ob_newsym->w;
		data2 += ob2->ob_newsym->w;
	}
}


static void scoretarg(OBJECTS *obp, int score)
{
	register OBJECTS *ob;

	ob = obp;
	if (playmode != PLAYMODE_ASYNCH) {
		if (ob->ob_clr == 1)
			nobjects[0].ob_score -= score;
		else
			nobjects[0].ob_score += score;
		dispscore(&nobjects[0]);
	} else {
		nobjects[2 - ob->ob_clr].ob_score += score;
		dispscore(&nobjects[2 - ob->ob_clr]);
	}
}


static BOOL scorepenalty(obtype_t ttype, OBJECTS * ob, int score)
{
	register OBJECTS *obt;

	obt = ob;
	if (ttype == SHOT || ttype == BOMB || ttype == MISSILE
	    || (ttype == PLANE
		&& (obt->ob_state == FLYING
		    || obt->ob_state == WOUNDED
		    || (obt->ob_state == FALLING
			&& obt->ob_hitcount == FALLCOUNT))
		&& !obt->ob_athome)) {
		scoretarg(obt, score);
		return TRUE;
	}
	return FALSE;
}




static int crtdepth[8] = { 1, 2, 2, 3, 3, 2, 2, 1 };

static void crater(OBJECTS * ob)
{
	register int i, x, y, ymin, ymax;
	int xmin, xmax;

	xmin = ob->ob_x + (ob->ob_newsym->w - 8) / 2;
	xmax = xmin + 7;

	for (x = xmin, i = 0; x <= xmax; ++x, ++i) {
		ymax = ground[x];
		ymin = ymax - crtdepth[i] +1;
		y = orground[x] - 20;
		if (y < 20)
			y = 20;
		if (ymin <= y)
			ymin = y + 1;
		ground[x] = ymin - 1;
	}
	forcdisp = TRUE;
}



// sdh -- renamed this to swkill to remove possible conflicts with
// the unix kill() function

static void swkill(OBJECTS * ob1, OBJECTS * ob2)
{
	register OBJECTS *ob, *obt;
	register int i;
	obtype_t ttype;
	obstate_t state;

	ob = ob1;
	obt = ob2;
	ttype = obt ? obt->ob_type : GROUND;
	if ((ttype == BIRD || ttype == FLOCK)
	    && ob->ob_type != PLANE)
		return;

	switch (ob->ob_type) {

	case BOMB:
	case MISSILE:
		initexpl(ob, 0);
		ob->ob_life = -1;
		if (!obt)
			crater(ob);
		stopsound(ob);
		return;

	case SHOT:
		ob->ob_life = 1;
		return;

	case STARBURST:
		if (ttype == MISSILE || ttype == BOMB || !obt)
			ob->ob_life = 1;
		return;

	case EXPLOSION:
		if (!obt) {
			ob->ob_life = 1;
			stopsound(ob);
		}
		return;

	case TARGET:
		if (ob->ob_state != STANDING)
			return;
		if (ttype == EXPLOSION || ttype == STARBURST)
			return;

		if (ttype == SHOT) {
			ob->ob_hitcount += TARGHITCOUNT;
			if (ob->ob_hitcount
			    <= (TARGHITCOUNT * (gamenum + 1)))
			return;
		}

		ob->ob_state = FINISHED;
		initexpl(ob, 0);

		scoretarg(ob, ob->ob_orient == 2 ? 200 : 100);

		if (numtarg[ob->ob_clr - 1] > 0) {
			--numtarg[ob->ob_clr - 1];
			if (numtarg[ob->ob_clr - 1] <= 0)
				endgame(ob->ob_clr);
		}

		return;

	case PLANE:
		state = ob->ob_state;

		if (state == CRASHED || state == GHOSTCRASHED)
			return;

		if (endsts[ob->ob_index] == WINNER)
			return;

		if (ttype == STARBURST
		    || (ttype == BIRD && ob->ob_athome))
			return;

		if (!obt) {
			if (state == FALLING) {
				stopsound(ob);
				initexpl(ob, 1);
				crater(ob);
			} else if (state < FINISHED) {
				scorepln(ob);
				initexpl(ob, 1);
				crater(ob);
			}

			crashpln(ob);
			return;
		}

		if (state >= FINISHED)
			return;

		if (state == FALLING) {
			if (ob->ob_index == player) {
				if (ttype == SHOT)
					swwindshot();
				else if (ttype == BIRD || ttype == FLOCK)
					swsplatbird();
			}
			return;
		}

		if (ttype == SHOT || ttype == BIRD
		    || ttype == OX || ttype == FLOCK) {
			if (ob->ob_index == player) {
				if (ttype == SHOT)
					swwindshot();
				else if (ttype == OX)
					swsplatox();
				else
					swsplatbird();
			}

			// sdh 28/10/2001: option to disable wounded planes

			if (conf_wounded) {
				if (state == FLYING) {
					ob->ob_state = WOUNDED;
					return;
				}
				if (state == STALLED) {
					ob->ob_state = WOUNDSTALL;
					return;
				}
			}
		} else {
			initexpl(ob, 1);
			if (ttype == PLANE) {
				collxadj = -collxadj;
				collyadj = -collyadj;
				collsdx[collptr]
				    = ((ob->ob_dx + obt->ob_dx) >> 1)
				    + collxadj;
				collsdy[collptr]
				    = ((ob->ob_dy + obt->ob_dy) >> 1)
				    + collyadj;
				collsno[collptr++] = ob;
			}
		}

		hitpln(ob);
		scorepln(ob);
		return;

	case BIRD:
		ob->ob_life = scorepenalty(ttype, obt, 25) ? -1 : -2;
		return;

	case FLOCK:
		if (ttype != FLOCK && ttype != BIRD
		    && ob->ob_state == FLYING) {
			for (i = 0; i < 8; ++i)
				initbird(ob, i);
			ob->ob_life = -1;
			ob->ob_state = FINISHED;
		}
		return;

	case OX:
		if (ob->ob_state != STANDING)
			return;
		if (ttype == EXPLOSION || ttype == STARBURST)
			return;
		scorepenalty(ttype, obt, 200);
		ob->ob_state = FINISHED;
		return;
	default:
		return;
	}
}


void swcollsn()
{
	register OBJECTS *ob, *obp, **obkd, **obkr;
	register int xmax, ymin, ymax, i;
	obtype_t otype;
	int prevx1, prevx2;

	collptr = killptr = 0;
	collxadj = 2;
	collyadj = 1;
	if (countmove & 1) {
		collxadj = -collxadj;
		collyadj = -collyadj;
	}

	prevx1 = topobj.ob_x;
	for (ob = topobj.ob_xnext; ob != &botobj; ob = ob->ob_xnext) {
		prevx2 = prevx1 = ob->ob_x;

		xmax = ob->ob_x + ob->ob_newsym->w - 1;
		ymax = ob->ob_y;
		ymin = ymax - ob->ob_newsym->h + 1;

		for (obp = ob->ob_xnext;
		     obp != &botobj && obp->ob_x <= xmax;
		     obp = obp->ob_xnext) {
			prevx2 = obp->ob_x;

			if (obp->ob_y >= ymin
			    && (obp->ob_y - obp->ob_newsym->h + 1) <= ymax)
				colltest(ob, obp);
		}

		otype = ob->ob_type;

		if ((otype == PLANE
		     && ob->ob_state != FINISHED
		     && ob->ob_state != WAITING
		     && ob->ob_y < (ground[ob->ob_x + 8] + 24))
		    || ((otype == BOMB || otype == MISSILE)
			&& ob->ob_y < (ground[ob->ob_x + 4] + 12)))
			tstcrash(ob);
	}

	obkd = killed;
	obkr = killer;
	for (i = 0; i < killptr; ++i, ++obkd, ++obkr)
		swkill(*obkd, *obkr);

	obkd = collsno;
	for (i = 0; i < collptr; ++i, ++obkd) {
		ob = *obkd;
		ob->ob_dx = collsdx[i];
		ob->ob_dy = collsdy[i];
	}
}

void tstcrash(OBJECTS * obp)
{
	register sopsym_t *sym = obp->ob_newsym;
	register int x, y;

	for (x=0; x<sym->w; ++x) {
		y = obp->ob_y - ground[x + obp->ob_x];

		// out of range?

		if (y >= sym->h)
			continue; 

		// check for collision at this point

		if (y < 0 || sym->data[y * sym->w + x]) {

			// collision!

			if (killptr < 2 * MAX_OBJS) {
				killed[killptr] = obp;
				killer[killptr] = NULL;
				++killptr;
			}

			return;
		}
	}
}


void scorepln(OBJECTS * ob)
{
	scoretarg(ob, 50);
}


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.11  2004/10/20 18:12:57  fraggle
// Fix incorrect endgames when numtargs < 0
//
// Revision 1.10  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.9  2004/10/15 18:51:24  fraggle
// Fix the map. Rename dispworld to dispmap as this is what it really does.
//
// Revision 1.8  2004/10/15 17:52:31  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.7  2004/10/15 17:23:32  fraggle
// Restore HUD splats
//
// Revision 1.6  2004/10/15 16:39:32  fraggle
// Unobfuscate some parts
//
// Revision 1.5  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.4  2003/06/08 03:41:41  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.3.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.3  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.2  2003/04/05 22:31:29  fraggle
// Remove PLAYMODE_MULTIPLE and swnetio.c
//
// Revision 1.1.1.1  2003/02/14 19:03:09  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 28/07/2002: removed old collision detection code
// sdh 28/06/2002: new collision detection code: look at the sprite data
//                 rather than drawing to the screen. old code is still there
//                 under a #define but will eventually be removed.
// sdh 27/06/2002: move to new sopsym_t for symbols
// sdh 28/10/2001: option to disable wounded planes
// sdh 24/10/2001: fix score display, fix auxdisp buffer
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by
//                 hand to make more readable
// sdh 19/10/2001: removed externs, these are now in headers
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-04-05        Missile and starburst support
// 87-03-31        Missiles.
// 87-03-13        Splatted bird symbol.
// 87-03-12        More than 1 bullet to kill target.
// 87-03-12        Wounded airplanes.
// 87-03-11        No explosion on bird-plane collision
// 87-03-09        Microsoft compiler.
// 84-10-31        Atari
// 84-06-12        PCjr Speed-up
// 84-02-02        Development
//
//---------------------------------------------------------------------------
