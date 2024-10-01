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
//        swcollsn -      SW collision resolution
//

#include "sw.h"
#include "swcollsn.h"
#include "swend.h"
#include "swinit.h"
#include "swmain.h"
#include "swmove.h"
#include "swsound.h"
#include "swsplat.h"

static void tstcrash(OBJECTS *obp);
static void blast_target(OBJECTS *ob, obtype_t type);

static OBJECTS *killed[MAX_OBJS*2], *killer[MAX_OBJS*2];
static int killptr;

static int collsdx[MAX_PLYR];
static int collsdy[MAX_PLYR];
static OBJECTS *collsno[MAX_PLYR];
static int collptr;
static int collxadj, collyadj;


//#define COLL_DEBUG

static void colltest(OBJECTS * ob1, OBJECTS * ob2)
{
	int x, y;
	int x1, y1, x2, y2;
	int w, h;
	unsigned char *data1, *data2;

	if ((ob1->ob_type == PLANE && ob1->ob_state >= FINISHED)
	 || (ob2->ob_type == PLANE && ob2->ob_state >= FINISHED)
	 || (ob1->ob_type == EXPLOSION && ob2->ob_type == EXPLOSION)) {
		return;
	}

	// (x1, y1) are the coords of the area we are testing in ob1
	// (x2, y2) are the coords of the area in ob2
	// (w, h) is the size of the area

	// x:

	if (ob1->ob_x < ob2->ob_x) {
		x1 = ob2->ob_x - ob1->ob_x;
		x2 = 0;
		w = ob1->ob_newsym->w - x1;
		if (w > ob2->ob_newsym->w) {
			w = ob2->ob_newsym->w;
		}
	} else {
		x1 = 0;
		x2 = ob1->ob_x - ob2->ob_x;
		w = ob2->ob_newsym->w - x2;
		if (w > ob1->ob_newsym->w) {
			w = ob1->ob_newsym->w;
		}
	}

	// no intersection?

	if (w <= 0) {
		return;
	}

	// y:

	if (ob1->ob_y < ob2->ob_y) {
		y1 = 0;
		y2 = ob2->ob_y - ob1->ob_y;
		h = ob2->ob_newsym->h - y2;
		if (h > ob1->ob_newsym->h) {
			h = ob1->ob_newsym->h;
		}
	} else {
		y1 = ob1->ob_y - ob2->ob_y;
		y2 = 0;
		h = ob1->ob_newsym->h - y1;
		if (h > ob2->ob_newsym->h) {
			h = ob2->ob_newsym->h;
		}
	}

	// no intersection?

	if (h <= 0) {
		return;
	}

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


static OBJECTS *
get_score_obj(OBJECTS *ob, int *reverse)
/* Determine the object that receives a score if 'ob' is destriyed, and whether the score
** should be subtracted rather than added (iff reverse nonzero) */
{
	OBJECTS *retval;
	*reverse = 0;

	if (playmode != PLAYMODE_ASYNCH) {
		retval = planes[0];
		if (ob->ob_clr == 1) {
			*reverse = 1;
		}
	} else
		retval = planes[2 - ob->ob_clr];

	return retval;
}

static void scoretarg(OBJECTS *obp, int score)
{
	OBJECTS *ob;
	int reverse_score;

	ob = get_score_obj(obp, &reverse_score);
	if (reverse_score) {
		ob->ob_score.score -= score;
	} else {
		ob->ob_score.score += score;
	}
}


static bool scorepenalty(obtype_t ttype, OBJECTS * ob, int score)
{
	OBJECTS *obt;

	obt = ob;
	if (ttype == SHOT || ttype == BOMB || ttype == MISSILE
	    || (ttype == PLANE
		&& (obt->ob_state == FLYING
		    || obt->ob_state == WOUNDED
		    || (obt->ob_state == FALLING
			&& obt->ob_hitcount == FALLCOUNT))
		&& !obt->ob_athome)) {
		scoretarg(obt, score);
		return true;
	}
	return false;
}




static int crtdepth[8] = { 1, 2, 2, 3, 3, 2, 2, 1 };

static void crater(OBJECTS * ob)
{
	int i, x, y, ymin, ymax;
	int xmin, xmax;

	xmin = ob->ob_x + (ob->ob_newsym->w - 8) / 2;
	xmax = xmin + 7;

	for (x = xmin, i = 0; x <= xmax; ++x, ++i) {
		ymax = ground[x];
		ymin = ymax - crtdepth[i] +1;
		y = currgame->gm_ground[x] - 20;
		if (y < 20) {
			y = 20;
		}
		if (ymin <= y) {
			ymin = y + 1;
		}
		ground[x] = ymin - 1;
	}
}

/* Determine whether the parameter is a shot and hasn't moved very much yet;
** Used to avoid having planes hitting themselves */

static int
is_young_shot(OBJECTS *ob)
{
	return (ob && ob->ob_type == SHOT && ob->ob_life >= BULLIFE - 1);
}

static void swkill(OBJECTS * ob1, OBJECTS * ob2)
{
	OBJECTS *ob, *obt;
	int i;
	obtype_t ttype;
	obstate_t state;

	ob = ob1;
	obt = ob2;
	ttype = obt ? obt->ob_type : GROUND;
	if ((ttype == BIRD || ttype == FLOCK) && ob->ob_type != PLANE) {
		return;
	}

	switch (ob->ob_type) {

	case BOMB:
	case MISSILE:
		initexpl(ob, 0);
		ob->ob_life = -1;
		if (!obt) {
			crater(ob);
		}
		stopsound(ob);
		return;

	case SHOT:
		/* cr 2005-04-28: Don't stop the shot if it just
		 * launched from its presumed originator */

		if (!(obt && obt->ob_type == PLANE && is_young_shot(ob))) {
			ob->ob_life = 1;
		}
		return;

	case STARBURST:
		if (ttype == MISSILE || ttype == BOMB || !obt) {
			ob->ob_life = 1;
		}
		return;

	case EXPLOSION:
		if (!obt) {
			ob->ob_life = 1;
			stopsound(ob);
		}
		return;

	case TARGET:
		if (ob->ob_state != STANDING) {
			return;
		}
		if (ttype == EXPLOSION || ttype == STARBURST) {
			return;
		}

		if (ttype == SHOT) {
			ob->ob_hitcount += TARGHITCOUNT;
			if (ob->ob_hitcount <= (TARGHITCOUNT * (gamenum + 1))) {
				return;
			}
		}

		ob->ob_state = FINISHED;
		ob->ob_onmap = false;
		initexpl(ob, 0);

		blast_target(ob, ttype);
		scoretarg(ob, ob->ob_orient == 2 ? 200 : 100);

		--numtarg[ob->ob_clr - 1];
		if (numtarg[ob->ob_clr - 1] <= 0
		 && (playmode == PLAYMODE_ASYNCH || ob->ob_clr == 2)) {
			endgame(ob->ob_clr);
		}

		return;

	case PLANE:
		state = ob->ob_state;

		/* cr 2005-04-28: Avoid having planes hit themselves */

		if (is_young_shot(obt)) {
			return;	
		}

		if (state == CRASHED) {
			return;
		}

		if (ob->ob_endsts == WINNER) {
			return;
		}

		if (ttype == STARBURST
		    || (ttype == BIRD && ob->ob_athome)) {
			return;
		}

		if (!obt) {
			if (state == FALLING) {
				stopsound(ob);
				initexpl(ob, 1);
				crater(ob);
			} else if (state < FINISHED) {
				scorepln(ob, ttype);
				initexpl(ob, 1);
				crater(ob);
			}

			crashpln(ob);
			return;
		}

		if (state >= FINISHED) {
			return;
		}

		if (state == FALLING) {
			if (ob == consoleplayer) {
				if (ttype == SHOT) {
					swwindshot();
				} else if (ttype == OX) {
					swsplatox();
				} else if (ttype == BIRD || ttype == FLOCK) {
					swsplatbird();
				}
			}
			return;
		}

		if (ttype == SHOT || ttype == BIRD
		    || ttype == OX || ttype == FLOCK) {
			if (ob == consoleplayer) {
				if (ttype == SHOT) {
					swwindshot();
				} else if (ttype == OX) {
					swsplatox();
				} else {
					swsplatbird();
				}
			}

			// Wounding is only allowed if the player is actually
			// in the air. If an enemy plane scores a hit on a
			// player sitting on the runway, that is a successful
			// raid; otherwise, the damage would be repaired
			// immediately.
			if (conf_wounded && !ob->ob_athome) {
				if (ttype == SHOT) {
					ob->ob_flightscore.combatwound = true;
				}
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
		scorepln(ob, ttype);
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
		if (ob->ob_state != STANDING) {
			return;
		}
		if (ttype == EXPLOSION || ttype == STARBURST) {
			return;
		}
		scorepenalty(ttype, obt, 200);
		ob->ob_state = FINISHED;
		return;
	default:
		return;
	}
}


void swcollsn(void)
{
	OBJECTS *ob, *obp, **obkd, **obkr;
	int xmax, ymin, ymax, i;
	obtype_t otype;

	collptr = killptr = 0;
	collxadj = 2;
	collyadj = 1;
	if (countmove & 1) {
		collxadj = -collxadj;
		collyadj = -collyadj;
	}

	for (ob = topobj.ob_xnext; ob != &botobj; ob = ob->ob_xnext) {
		xmax = ob->ob_x + ob->ob_newsym->w - 1;
		ymax = ob->ob_y;
		ymin = ymax - ob->ob_newsym->h + 1;

		for (obp = ob->ob_xnext;
		     obp != &botobj && obp->ob_x <= xmax;
		     obp = obp->ob_xnext) {

			if (obp->ob_y >= ymin
			    && (obp->ob_y - obp->ob_newsym->h + 1) <= ymax) {
				colltest(ob, obp);
			}
		}

		otype = ob->ob_type;

		if ((otype == PLANE
		     && ob->ob_state != FINISHED
		     && ob->ob_state != WAITING
		     && ob->ob_y < (ground[ob->ob_x + 8] + 24))
		    || ((otype == BOMB || otype == MISSILE)
			&& ob->ob_y < (ground[ob->ob_x + 4] + 12))) {
			tstcrash(ob);
		}
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

static void tstcrash(OBJECTS *obp)
{
	sopsym_t *sym = obp->ob_newsym;
	int x, y;

	for (x=0; x<sym->w; ++x) {
		y = obp->ob_y - ground[x + obp->ob_x];

		// out of range?

		if (y >= sym->h) {
			continue;
		}

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


/* Determine valour factor */

static int compute_valour(OBJECTS *ob)
{
	int reverse;
	OBJECTS *so = get_score_obj(ob, &reverse);
	int x_home;
	int distance;
	int valour = 0;
	int fuelfraction;

	if (reverse) {
		return 0;
	}

	x_home = so->ob_original_ob->x;

	distance = abs(x_home - so->ob_x);

	if (distance < 500) {
		valour = 0;
	} else {
		valour = (distance - 500) / 350;
	}

	if (ob->ob_life > 0) {
		fuelfraction = (MAXFUEL / ob->ob_life);
	} else {
		fuelfraction = 1000;
	}

	if (fuelfraction > 9) {
		valour++;
	}

	if (so->ob_state == WOUNDED
	    || so->ob_state == WOUNDSTALL) {
		valour = (valour + 1) * 3;
	} else {
		valour *= 2;
	}

	return valour;
}

static void blast_target(OBJECTS *ob, obtype_t type)
{
	int reverse;
	OBJECTS *so = get_score_obj(ob, &reverse);

	if (reverse) {
		return;
	}

	if (type == BOMB || type == SHOT || type == MISSILE || type == PLANE) {
		so->ob_flightscore.killscore += 4;
		so->ob_flightscore.valour += 3 * compute_valour (ob);
	}
}

void scorepln(OBJECTS * ob, obtype_t type)
{
	int had_taken_off = ob->ob_life < (MAXFUEL - (MAXFUEL / 100));

	scoretarg(ob, 50);

	if (type == BOMB || type == SHOT || type == MISSILE || type == PLANE) {
		int reverse;
		OBJECTS *scobj = get_score_obj(ob, &reverse);
		if (!reverse) {
			if (had_taken_off) {
				if (type != PLANE) {
					scobj->ob_flightscore.planekills++;
				}
				scobj->ob_flightscore.valour +=
					4 * (2 + compute_valour (ob));
			}

			scobj->ob_flightscore.killscore += 3;
		}
	}
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
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
