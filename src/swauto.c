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
//        swauto   -      SW control of computer player
//

#include <assert.h>

#include "sw.h"
#include "swauto.h"
#include "swinit.h"
#include "swmain.h"
#include "swobject.h"

static bool correction;		/*  Course correction flag        */
static OBJECTS obs;		/*  Saved computer object         */
static int courseadj;		/*  Course adjustment             */

static int shoot(OBJECTS *obt)
{
	static OBJECTS obsp, obtsp;
	int obx, oby, obtx, obty;
	int nspeed, nangle;
	int rprev;
	int r, i;

	copyobj(&obsp, &obs);
	copyobj(&obtsp, obt);
	nspeed = obsp.ob_speed + BULSPEED;
	setdxdy(&obsp,
		nspeed * COS(obsp.ob_angle),
		nspeed * SIN(obsp.ob_angle));
	obsp.ob_x += SYM_WDTH / 2;
	obsp.ob_y -= SYM_HGHT / 2;

	nangle = obtsp.ob_angle;
	nspeed = obtsp.ob_speed;
	rprev = NEAR;

	for (i = 0; i < BULLIFE; ++i) {
		movexy(&obsp, &obx, &oby);
		if (obtsp.ob_state == FLYING || obtsp.ob_state == WOUNDED) {

			r = obtsp.ob_flaps;

			if (r) {
				if (obtsp.ob_orient) {
					nangle -= r;
				} else {
					nangle += r;
				}
				nangle = (nangle + ANGLES) % ANGLES;
				setdxdy(&obtsp,
					nspeed * COS(nangle),
					nspeed * SIN(nangle));
			}
		}

		movexy(&obtsp, &obtx, &obty);
		r = range(obx, oby, obtx, obty);
		if (r < 0 || r > rprev) {
			return 0;
		}
		if (obx >= obtx
		    && obx <= (obtx + SYM_WDTH - 1)
		    && oby <= obty
		    && oby >= (obty - SYM_HGHT + 1)) {
			return 1 + (i > (BULLIFE / 3));
		}

	}

	return 0;
}

static bool within_home_range(OBJECTS *ob, int x, int y)
{
	const original_ob_t *orig_ob = ob->ob_original_ob;

	return abs(x - orig_ob->x) < HOME
	    && abs(y - ob->ob_orig_y) < HOME;
}

static bool is_target(OBJECTS *ob)
{
	return ob->ob_type == TARGET || ob->ob_type == OX;
}

static bool tstcrash2(OBJECTS *ob, int x, int y, int alt, int dy)
{
	OBJECTS *obt;
	int xl, xr, yt, lookahead = 3;

	if (alt > 50) {
		return false;
	}

	// If we're heading home and this will be the last step here, we
	// don't need to look three tics ahead. Otherwise at high speeds the
	// autopilot can end up endlessly circling the runway, afraid any
	// downward movement at all will lead to a crash.
	if (ob->ob_home && within_home_range(ob, x, y)) {
		lookahead = 1;
	}

	// If we're going to get too low, don't aim downwards. This is
	// expressed in terms of the speed we will be heading towards
	// the ground; if we are likely to crash within three tics, we
	// should try to pull away. This approach is independent of the
	// speed we are travelling at, so it works at the higher levels
	// where the plane travels very fast.
	// The behavior here has been fixed compared to the original
	// Author's Edition release. It used to be that planes would
	// sometimes try to take off by flying straight upwards without
	// full throttle.
	if (alt + dy * lookahead / 256 < 8) {
		return true;
	}

	// This is unnecessarily complicated really, but preserves the
	// logic of the previous version of tstcrash2(). It can probably
	// be simplified.
	xl = ob->ob_x - 32 - gmaxspeed;
	if (x - 32 > xl) {
		xl = x - 32;
	}
	xr = ob->ob_x + 32 + gmaxspeed;
	if (x + 32 < xr) {
		xr = x + 32;
	}

	obt = ob;
	while (obt->ob_xprev != NULL && obt->ob_xprev->ob_x >= xl) {
		obt = obt->ob_xprev;
	}


	for (; obt->ob_xnext != NULL; obt = obt->ob_xnext) {
		if (!is_target(obt) || obt->ob_x < xl) {
			continue;
		}
		if (obt->ob_x > xr) {
			return false;
		}
		yt = obt->ob_y + (obt->ob_state == STANDING ? 16 : 8);
		if (y <= yt) {
			return true;
		}
	}
	return false;
}

int aim(OBJECTS *ob, int ax, int ay, OBJECTS *obt, bool longway)
{
	int r, rmin, i, n=0;
	int x, y, dx, dy, nx, ny;
	int nangle, nspeed;
	static int cflaps[3] = { 0, -1, 1 };
	static int crange[3], ccrash[3], calt[3];

	correction = false;

	if (plane_is_stalled(ob->ob_state)
	 && ob->ob_angle != (3 * ANGLES / 4)) {
		ob->ob_flaps = -1;
		ob->ob_accel = MAX_THROTTLE;
		return 0;
	}

	x = ob->ob_x;
	y = ob->ob_y;

	dx = x - ax;

	if (abs(dx) > 160) {
		if (ob->ob_dx && (dx < 0) == (ob->ob_dx < 0)) {
			if (!ob->ob_hitcount) {
				ob->ob_hitcount = (y > (MAX_Y - 50)) ? 2 : 1;
			}
			return (aim(ob, x, ob->ob_hitcount == 1
				    ? (y + 25) : (y - 25), NULL, true));
		}
		ob->ob_hitcount = 0;
		return (aim(ob, x + (dx < 0 ? 150 : -150),
			    (y + 100 > MAX_Y - 50 - courseadj)
			        ? MAX_Y - 50 - courseadj
			        : y + 100,
			    NULL, true));
	} else {
		if (!longway) {
			ob->ob_hitcount = 0;
		}
	}

	if (ob->ob_speed) {

		correction = dy = y - ay;

		if (correction && abs(dy) < 6) {
			if (dy < 0) {
				++y;
			} else {
				--y;
			}
			ob->ob_y = y;
		} else {
			correction = dx;
			if (correction && abs(dx) < 6) {
				if (dx < 0) {
					++x;
				} else {
					--x;
				}
				ob->ob_x = x;
				updateobjpos(ob);
			}
		}
	}

	copyobj(&obs, ob);

	nspeed = obs.ob_speed + 1;
	if (nspeed > gmaxspeed && obs.ob_type == PLANE) {
		nspeed = gmaxspeed;
	} else if (nspeed < gminspeed) {
		nspeed = gminspeed;
	}

	for (i = 0; i < 3; ++i) {
		nangle = (obs.ob_angle
			  + (obs.ob_orient ? -cflaps[i] : cflaps[i])
			  + ANGLES) % ANGLES;
		setdxdy(&obs, nspeed * COS(nangle), nspeed * SIN(nangle));
		movexy(&obs, &nx, &ny);
		crange[i] = range(nx, ny, ax, ay);
		calt[i] = ny - currgame->gm_ground[nx + 8];
		ccrash[i] = tstcrash2(ob, nx, ny, calt[i], nspeed * SIN(nangle));

		copyobj(&obs, ob);
	}


	if (obt) {
		i = shoot(obt);

		if (i) {
			// cr 2005-04-28: Resort to MG if
			//       missiles are disabled

			if (ob->ob_missiles && conf_missiles && i == 2) {
				ob->ob_mfiring = obt->ob_athome ? ob : obt;
			} else {
				ob->ob_firing = obt;
			}
		}
	}

	rmin = 32767;
	for (i = 0; i < 3; ++i) {
		r = crange[i];
		if (r >= 0 && r < rmin && !ccrash[i]) {
			rmin = r;
			n = i;
		}
	}

	if (rmin == 32767) {
		rmin = -32767;
		for (i = 0; i < 3; ++i) {
			r = crange[i];
			if (r < 0 && r > rmin && !ccrash[i]) {
				rmin = r;
				n = i;
			}
		}
	}

	if (ob->ob_speed < gminspeed) {
		ob->ob_accel = MAX_THROTTLE;
	}

	if (rmin == -32767) {
		if (ob->ob_accel) {
			--ob->ob_accel;
		}

		n = 0;

		dy = calt[0];

		if (calt[1] > calt[0]) {
			dy = calt[1];
			n = 1;
		}
		if (calt[2] > dy) {
			n = 2;
		}
	} else {
		if (ob->ob_accel < MAX_THROTTLE) {
			++ob->ob_accel;
		}
	}

	ob->ob_flaps = cflaps[n];
	if (ob->ob_type == PLANE && !ob->ob_flaps) {
		if (ob->ob_speed) {
			ob->ob_orient = ob->ob_dx < 0;
		}
	}
	return 0;
}



int gohome(OBJECTS *ob)
{
	if (ob->ob_athome) {
		return 0;
	}

	courseadj = ((countmove & 0x001F) < 16) << 4;
	if (within_home_range(ob, ob->ob_x, ob->ob_y)) {
		if (plyrplane) {
			initplyr(ob);
			initdisp(true);
		} else if (compplane) {
			initcomp(ob);
		} else {
			initpln(ob);
		}
		return 0;
	}

	/* When wounded, only move every other tic */

	if (ob->ob_state == WOUNDED && (countmove & 1)) {
		return 0;
	} else {
		return aim(ob, ob->ob_original_ob->x, ob->ob_orig_y,
		           NULL, false);
	}
}




static void cruise(OBJECTS *ob)
{
	int orgx;

	courseadj = ((countmove & 0x001F) < 16) << 4;
	orgx = ob->ob_original_ob->x;
	aim(ob, courseadj +
		(orgx < (currgame->gm_max_x / 3) ? (currgame->gm_max_x / 3) :
		 orgx > (2 * currgame->gm_max_x / 3) ?
		     (2 * currgame->gm_max_x / 3) : orgx),
		MAX_Y - 50 - (courseadj >> 1), NULL, false);
}

static void attack(OBJECTS *obp, OBJECTS *ob)
{
	courseadj = ((countmove & 0x001F) < 16) << 4;
	if (ob->ob_speed) {
		aim(obp,
		    ob->ob_x - ((CLOSE * COS(ob->ob_angle)) >> 8),
		    ob->ob_y - ((CLOSE * SIN(ob->ob_angle)) >> 8), ob, false);
	} else {
		aim(obp, ob->ob_x, ob->ob_y + 4, ob, false);
	}
}


void swauto(OBJECTS *ob)
{
	if (ob->ob_target != NULL) {
		attack(ob, ob->ob_target);
	} else if (!ob->ob_athome) {
		cruise(ob);
	}

	ob->ob_target = NULL;
}


int range(int x, int y, int ax, int ay)
{
	int dx, dy;
	int t;

	dy = abs(y - ay);
	dy += dy >> 1;
	dx = abs(x - ax);

	if (dx < 125 && dy < 125) {
		return dx * dx + dy * dy;
	}

	if (dx < dy) {
		t = dx;
		dx = dy;
		dy = t;
	}

	return -((7 * dx + (dy << 2)) >> 3);
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by
// 		   hand to make more readable
// sdh 19/10/2001: removed all externs, this is now in headers
// 		   some static functions shuffled around to shut
//                 up the compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-04-09        Don't attack until closer.
// 87-04-06        Computer plane avoiding oxen.
// 87-04-05        Missile and starburst support
// 87-03-31        Missiles.
// 87-03-12        Smarter shooting.
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 84-10-31        Atari
// 84-06-12        PCjr Speed-up
// 84-03-05        Development
//
