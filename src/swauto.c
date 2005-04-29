// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
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
//        swauto   -      SW control of computer player
//
//---------------------------------------------------------------------------

#include "sw.h"
#include "swauto.h"
#include "swground.h"
#include "swinit.h"
#include "swmain.h"
#include "swutil.h"

static BOOL correction;		/*  Course correction flag        */
static OBJECTS obs;		/*  Saved computer object         */
static int courseadj;		/*  Course adjustment             */

int shoot(OBJECTS *obt)
{
	static OBJECTS obsp, obtsp;
	int obx, oby, obtx, obty;
	int nspeed, nangle;
	int rprev;
	register int r, i;

	obsp = obs;
	obtsp = *obt;
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
				if (obtsp.ob_orient)
					nangle -= r;
				else
					nangle += r;
				nangle = (nangle + ANGLES) % ANGLES;
				setdxdy(&obtsp, 
					nspeed * COS(nangle),
					nspeed * SIN(nangle));
			}
		}

		movexy(&obtsp, &obtx, &obty);
		r = range(obx, oby, obtx, obty);
		if (r < 0 || r > rprev)
			return 0;
		if (obx >= obtx
		    && obx <= (obtx + SYM_WDTH - 1)
		    && oby <= obty
		    && oby >= (obty - SYM_HGHT + 1))
			return 1 + (i > (BULLIFE / 3));

	}

	return 0;
}



/*  sdh -- this is standard c


    abs( x )
    int     x;
    {
    return( ( x < 0 ) ? -x : x );
    }
*/

static int tl, tr;

static void cleartargs()
{
	tl = -2;
}

static void testtargs(int x, int y)
{
	register int i, xl, xr;

	xl = x - 32 - gmaxspeed;
	xr = x + 32 + gmaxspeed;

	tl = -1;
	tr = 0;
	for (i = 0; i < (MAX_TARG + MAX_OXEN); ++i)
		if (targets[i] && targets[i]->ob_x >= xl) {
			tl = i;
			break;
		}

	if (tl == -1)
		return;

	for (; i < MAX_TARG + MAX_OXEN 
	       && targets[i]
	       && targets[i]->ob_x < xr; ++i);

	tr = i - 1;
}



// sdh: changed to tstcrash2 to stop conflicts with the other 
// function with the same name in swcollsn.c

static BOOL tstcrash2(OBJECTS *obp, int x, int y, int alt)
{
	register OBJECTS *ob;
	register int i, xl, xr, xt, yt;

	if (alt > 50)
		return FALSE;

	if (alt < 22)
		return TRUE;

	ob = obp;
	if (tl == -2)
		testtargs(ob->ob_x, ob->ob_y);

	xl = x - 32;
	xr = x + 32;

	for (i = tl; i <= tr; ++i) {
		ob = targets[i];
		xt = ob->ob_x;

		if (xt < xl)
			continue;
		if (xt > xr)
			return FALSE;
		yt = ob->ob_y + (ob->ob_state == STANDING ? 16 : 8);
		if (y <= yt)
			return TRUE;
	}
	return FALSE;
}

int aim(OBJECTS *obo, int ax, int ay, OBJECTS *obt, BOOL longway)
{
	register OBJECTS *ob;
	register int r, rmin, i, n=0;
	int x, y, dx, dy, nx, ny;
	int nangle, nspeed;
	static int cflaps[3] = { 0, -1, 1 };
	static int crange[3], ccrash[3], calt[3];

	ob = obo;

	correction = FALSE;

	if ((ob->ob_state == STALLED || ob->ob_state == WOUNDSTALL)
	    && ob->ob_angle != (3 * ANGLES / 4)) {
		ob->ob_flaps = -1;
		ob->ob_accel = MAX_THROTTLE;
		return 0;
	}

	x = ob->ob_x;
	y = ob->ob_y;

	dx = x - ax;

	if (abs(dx) > 160) {
		if (ob->ob_dx && (dx < 0 == ob->ob_dx < 0)) {
			if (!ob->ob_hitcount)
				ob->ob_hitcount = (y > (MAX_Y - 50)) ? 2 : 1;
			return (aim(ob, x, ob->ob_hitcount == 1
				    ? (y + 25) : (y - 25), NULL, YES));
		}
		ob->ob_hitcount = 0;
		return (aim(ob, x + (dx < 0 ? 150 : -150),
			    (y + 100 > MAX_Y - 50 - courseadj)
		 		? MAX_Y - 50 - courseadj 
				: y + 100, 
			    NULL, YES));
	} else {
		if (!longway)
			ob->ob_hitcount = 0;
	}

	if (ob->ob_speed) {

		correction = dy = y - ay;

		if (correction && abs(dy) < 6) {
			if (dy < 0) 
				++y;
			else
				--y;
			ob->ob_y = y;
		} else {
			correction = dx;
			if (correction && abs(dx) < 6) {
				if (dx < 0)
					++x;
				else
					--x;
				ob->ob_x = x;
			}
		}
	}

	// sdh 16/11/2001: removed movmem

	obs = *ob;

	nspeed = obs.ob_speed + 1;
	if (nspeed > gmaxspeed && obs.ob_type == PLANE)
		nspeed = gmaxspeed;
	else if (nspeed < gminspeed)
		nspeed = gminspeed;

	cleartargs();
	for (i = 0; i < 3; ++i) {
		nangle = (obs.ob_angle
			  + (obs.ob_orient ? -cflaps[i] : cflaps[i])
			  + ANGLES) % ANGLES;
		setdxdy(&obs, nspeed * COS(nangle), nspeed * SIN(nangle));
		movexy(&obs, &nx, &ny);
		crange[i] = range(nx, ny, ax, ay);
		calt[i] = ny - orground[nx + 8];
		ccrash[i] = tstcrash2(ob, nx, ny, calt[i]);

		// sdh 16/11/2001: removed movmem
		obs = *ob;
	}


	if (obt) {
		i = shoot(obt);

		if (i) {
                        // cr 2005-04-28: Resort to MG if
                        //       missiles are disabled

			if (ob->ob_missiles && conf_missiles && i == 2) 
				ob->ob_mfiring = obt->ob_athome ? ob : obt;
			else
				ob->ob_firing = obt;
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

	if (ob->ob_speed < gminspeed)
		ob->ob_accel = MAX_THROTTLE;

	if (rmin == -32767) {
		if (ob->ob_accel)
			--ob->ob_accel;

		n = 0;

		dy = calt[0];

		if (calt[1] > calt[0]) {
			dy = calt[1];
			n = 1;
		}
		if (calt[2] > dy)
			n = 2;
	} else {
		if (ob->ob_accel < MAX_THROTTLE)
			++ob->ob_accel;
	}

	ob->ob_flaps = cflaps[n];
	if (ob->ob_type == PLANE && !ob->ob_flaps)
		if (ob->ob_speed)
			ob->ob_orient = ob->ob_dx < 0;

	return 0;
}



int gohome(OBJECTS *ob)
{
        OBJECTS *original_ob;

	if (ob->ob_athome)
		return 0;

	original_ob = &oobjects[ob->ob_index];

	courseadj = ((countmove & 0x001F) < 16) << 4;
	if (abs(ob->ob_x - original_ob->ob_x) < HOME
         && abs(ob->ob_y - original_ob->ob_y) < HOME) {
		if (plyrplane) {
			initplyr(ob);
			initdisp(YES);
		} else if (compplane) {
			initcomp(ob);
		} else {
			initpln(ob);
		}
		return 0;
	}

        /* When wounded, only move every other tic */

        if (ob->ob_state == WOUNDED && (countmove & 1))
                return 0;
        else
                return aim(ob, original_ob->ob_x, original_ob->ob_y, NULL, NO);
}




static void cruise(OBJECTS *ob)
{
	register int orgx;

	courseadj = ((countmove & 0x001F) < 16) << 4;
	orgx = oobjects[ob->ob_index].ob_x;
	aim(ob, courseadj +
		(orgx < (MAX_X / 3) ? (MAX_X / 3) :
		 orgx > (2 * MAX_X / 3) ? (2 * MAX_X / 3) : orgx),
		MAX_Y - 50 - (courseadj >> 1), NULL, NO);
}

void attack(OBJECTS *obp, OBJECTS *obt)
{
	register OBJECTS *ob;

	courseadj = ((countmove & 0x001F) < 16) << 4;
	ob = obt;
	if (ob->ob_speed)
		aim(obp,
		    ob->ob_x - ((CLOSE * COS(ob->ob_angle)) >> 8),
		    ob->ob_y - ((CLOSE * SIN(ob->ob_angle)) >> 8), ob, NO);
	else
		aim(obp, ob->ob_x, ob->ob_y + 4, ob, NO);
}


void swauto(OBJECTS *ob)
{
	if (compnear[ob->ob_index])
		attack(ob, compnear[ob->ob_index]);
	else if (!ob->ob_athome)
		cruise(ob);

	compnear[ob->ob_index] = NULL;
}


int range(int x, int y, int ax, int ay)
{
	register int dx, dy;
	register int t;

	dy = abs(y - ay);
	dy += dy >> 1;
	dx = abs(x - ax);

	if (dx < 125 && dy < 125)
		return dx * dx + dy * dy;

	if (dx < dy) {
		t = dx;
		dx = dy;
		dy = t;
	}

	return -((7 * dx + (dy << 2)) >> 3);
}



//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.8  2005/04/29 19:33:54  fraggle
// Move slowly when wounded and using autopilot
//
// Revision 1.7  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.6  2005/04/28 18:32:59  fraggle
// Fix bug with oobjects
// Remove unused "goinghome" flag
//
// Revision 1.5  2005/04/28 18:24:41  fraggle
// Fix 'home' key
//
// Revision 1.4  2005/04/28 10:42:48  fraggle
// Fix computer planes not firing when tailing player plane
//
// Revision 1.3  2004/10/20 19:00:01  fraggle
// Remove currobx, endsts variables
//
// Revision 1.2  2003/04/06 22:01:01  fraggle
// Fix compile warnings
//
// Revision 1.1.1.1  2003/02/14 19:03:08  fraggle
// Initial Sourceforge CVS import
//
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
//---------------------------------------------------------------------------

