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
//        swmove   -      SW move all objects and players
//
//---------------------------------------------------------------------------

#include "video.h"

#include "sw.h"
#include "swasynio.h"
#include "swauto.h"
#include "swcollsn.h"
#include "swdisp.h"
#include "swend.h"
#include "swground.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swmove.h"
#include "swobject.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swtitle.h"
#include "swutil.h"

static BOOL quit;

void swmove()
{
	register OBJECTS *ob, *obn;

	if (deltop) {
		delbot->ob_next = objfree;
		objfree = deltop;
		deltop = delbot = NULL;
	}

	++dispcnt;

	if (dispcnt >= keydelay)
		dispcnt = 0;

	ob = objtop;
	while (ob) {
		obn = ob->ob_next;
		ob->ob_drwflg = (*ob->ob_movef) (ob);
		ob = obn;
	}

	++countmove;
}


static void nearpln(OBJECTS * obp)
{
	register OBJECTS *ob, *obt, *obc;
	register int i, obx, obclr;

	ob = obp;
	obt = objtop + 1;

	obx = ob->ob_x;
	obclr = ob->ob_owner->ob_clr;

	for (i = 1; obt->ob_type == PLANE; ++i, ++obt) {
		if (obclr == obt->ob_owner->ob_clr)
			continue;

		// sdh: removed use of "equal" function to compare
		// function pointers (why???)

		if (obt->ob_drawf == dispcomp)

			if (playmode != PLAYMODE_COMPUTER
			    || (obx >= lcompter[i] && obx <= rcompter[i])) {
				obc = compnear[i];
				if (!obc
				    || abs(obx - obt->ob_x)
					< abs(obc->ob_x - obt->ob_x))
					compnear[i] = ob;
			}
	}
}




static void topup(int *counter, int max)
{
	if (*counter == max)
		return;
	if (max < 20) {
		if (!(countmove % 20)) {
			++*counter;
		}
	} else {
		*counter += max / 100;
	}
	if (*counter > max)
		*counter = max;
}


static void refuel(OBJECTS * obp)
{
	OBJECTS *ob;

	ob = obp;

	// sdh 26/10/2001: top up stuff, if anything happens update 
	// the guages (now a single function)
	// sdh 27/10/2001: fix refueling in parallel (was a single
	// set of ||'s and was being shortcircuited)

	topup(&ob->ob_life, MAXFUEL);
	topup(&ob->ob_rounds, MAXROUNDS);
	topup(&ob->ob_bombs, MAXBOMBS);
	topup(&ob->ob_missiles, MAXMISSILES);
	topup(&ob->ob_bursts, MAXBURSTS);
}



static int symangle(OBJECTS * ob)
{
	register int dx, dy;

	dx = ob->ob_dx;
	dy = ob->ob_dy;
	if (dx == 0)
		if (dy < 0)
			return 6;
		else if (dy > 0)
			return 2;
		else
			return 6;
	else if (dx > 0)
		if (dy < 0)
			return 7;
		else if (dy > 0)
			return 1;
		else
			return 0;
	else if (dy < 0)
		return 5;
	else if (dy > 0)
		return 3;
	else
		return 4;
}

BOOL moveplyr(OBJECTS * obp)
{
	register OBJECTS *ob;
	register BOOL rc;
	int multkey;

	compplane = FALSE;
	plyrplane = player == obp->ob_plrnum;

	ob = obp;

	endstat = consoleplayer->ob_endsts;

	if (endstat) {
		--endcount;
		if (endcount <= 0) {
			if (playmode != PLAYMODE_ASYNCH && !quit)
				swrestart();
			swend(NULL, YES);
		}
	}
	
	// get move command for this tic
	
	multkey = latest_player_commands[ob->ob_plrnum][countmove % MAX_NET_LAG];

	// Thanks to Kodath duMatri for fixing this :)

	if (conf_harrykeys && ob->ob_orient)
		if(multkey & (K_FLAPU | K_FLAPD))
			multkey ^= K_FLAPU | K_FLAPD;

	interpret(ob, multkey);

	/*
	if (dispcnt) {
		ob->ob_flaps = 0;
		ob->ob_bfiring = ob->ob_bombing = FALSE;
		ob->ob_mfiring = NULL;
	}*/

	if ((ob->ob_state == CRASHED || ob->ob_state == GHOSTCRASHED)
	    && ob->ob_hitcount <= 0) {

		// sdh: infinite lives in multiplayer mode

		if (playmode != PLAYMODE_ASYNCH)
			++ob->ob_crashcnt;

		if (endstat != WINNER
		    && (ob->ob_life <= QUIT
			|| (playmode != PLAYMODE_ASYNCH
			    && ob->ob_crashcnt >= MAXCRASH))) {
			if (!endstat)
				loser(ob);
		} else {
			initplyr(ob);
			initdisp(YES);
			if (endstat == WINNER) {
				if (ctlbreak())
					swend(NULL, YES);
				winner(ob);
			}
		}
	}

	return movepln(ob);
}




void interpret(OBJECTS * obp, int key)
{
	register OBJECTS *ob;
	register obstate_t state;

	ob = obp;
	ob->ob_flaps = 0;
	ob->ob_bombing = ob->ob_bfiring = 0;
	ob->ob_mfiring = ob->ob_firing = NULL;

	state = ob->ob_state;

	if (state != FLYING
	    && state != STALLED
	    && state != FALLING
	    && state != WOUNDED
	    && state != WOUNDSTALL
	    && state != GHOST
	    && state != GHOSTSTALLED)
		return;

	if (state != FALLING) {
		if (endstat) {
			if (endstat == LOSER && plyrplane)
				gohome(ob);
			return;
		}

		if (key & K_BREAK) {
			ob->ob_life = QUIT;
			ob->ob_home = FALSE;
			if (ob->ob_athome) {
				ob->ob_state = state = 
				    state >= FINISHED ? GHOSTCRASHED : CRASHED;
				ob->ob_hitcount = 0;
			}
			if (plyrplane)
				quit = TRUE;
		}

		if (key & K_HOME)
			if (state == FLYING || state == GHOST 
			    || state == WOUNDED)
				ob->ob_home = TRUE;
	}

	if ((countmove & 1)
	    || (state != WOUNDED && state != WOUNDSTALL)) {
		if (key & K_FLAPU) {
			++ob->ob_flaps;
			ob->ob_home = FALSE;
		}

		if (key & K_FLAPD) {
			--ob->ob_flaps;
			ob->ob_home = FALSE;
		}

		if (key & K_FLIP) {
			ob->ob_orient = !ob->ob_orient;
			ob->ob_home = FALSE;
		}

		if (key & K_DEACC) {
			if (ob->ob_accel)
				--ob->ob_accel;
			ob->ob_home = FALSE;
		}

		if (key & K_ACCEL) {
			if (ob->ob_accel < MAX_THROTTLE)
				++ob->ob_accel;
			ob->ob_home = FALSE;
		}
	}

	if ((key & K_SHOT) && state < FINISHED)
		ob->ob_firing = ob;

	if ((key & K_MISSILE) && state < FINISHED)
		ob->ob_mfiring = ob;

	if ((key & K_BOMB) && state < FINISHED)
		ob->ob_bombing = TRUE;

	if ((key & K_STARBURST) && state < FINISHED)
		ob->ob_bfiring = TRUE;

	if (key & K_SOUND)
		if (plyrplane) {
			if (soundflg) {
				sound(0, 0, NULL);
				swsound();
			}
			soundflg = !soundflg;
		}

	if (ob->ob_home)
		gohome(ob);
}

BOOL movecomp(OBJECTS * obp)
{
	register OBJECTS *ob;
	int rc;

	compplane = TRUE;
	plyrplane = FALSE;

	ob = obp;
	ob->ob_flaps = 0;
	ob->ob_bfiring = ob->ob_bombing = FALSE;
	ob->ob_mfiring = NULL;

	endstat = ob->ob_endsts;

	if (!dispcnt)
		ob->ob_firing = NULL;

	switch (ob->ob_state) {

	case WOUNDED:
	case WOUNDSTALL:
		if (countmove & 1)
			break;

	case FLYING:
	case STALLED:
		if (endstat) {
			gohome(ob);
			break;
		}
		if (!dispcnt)
			swauto(ob);
		break;

	case CRASHED:
		ob->ob_firing = NULL;
		if (ob->ob_hitcount <= 0 && !endstat)
			initcomp(ob);
		break;

	default:
		ob->ob_firing = NULL;
		break;
	}


	rc = movepln(ob);

	return rc;
}

static BOOL stallpln(OBJECTS * obp)
{
	register OBJECTS *ob;

	ob = obp;
	ob->ob_ldx = ob->ob_ldy = ob->ob_orient = ob->ob_dx = 0;
	ob->ob_angle = 7 * ANGLES / 8;
	ob->ob_speed = 0;
	ob->ob_dy = 0;
	ob->ob_hitcount = STALLCOUNT;
	ob->ob_state = 
		ob->ob_state >= GHOST ? GHOSTSTALLED :
		ob->ob_state == WOUNDED ? WOUNDSTALL : STALLED;
	ob->ob_athome = FALSE;

	return TRUE;
}



BOOL movepln(OBJECTS * obp)
{
	register OBJECTS *ob = obp;
	register int nangle, nspeed, limit, update;
	obstate_t state, newstate;
	int x, y, stalled;
	// int grv;

	// sdh 28/4/2002: aargh! char is not neccesarily signed char,
	// it seems. use int

	static signed int gravity[] = { 
		0, -1, -2, -3, -4, -3, -2, -1,
		0, 1, 2, 3, 4, 3, 2, 1
	};

	state = ob->ob_state;

	switch (state) {
	case FINISHED:
	case WAITING:
		return FALSE;

	case CRASHED:
	case GHOSTCRASHED:
		--ob->ob_hitcount;
		break;

	case FALLING:
		ob->ob_hitcount -= 2;
		if ((ob->ob_dy < 0) && ob->ob_dx) {
			if (ob->ob_orient ^ (ob->ob_dx < 0))
				ob->ob_hitcount -= ob->ob_flaps;
			else
				ob->ob_hitcount += ob->ob_flaps;
		}

		if (ob->ob_hitcount <= 0) {
			if (ob->ob_dy < 0) {
				if (ob->ob_dx < 0)
					++ob->ob_dx;
				else if (ob->ob_dx > 0)
					--ob->ob_dx;
				else
					ob->ob_orient = !ob->ob_orient;
			}

			if (ob->ob_dy > -10)
				--ob->ob_dy;
			ob->ob_hitcount = FALLCOUNT;
		}
		ob->ob_angle = symangle(ob) * 2;
		if (ob->ob_dy <= 0)
			initsound(ob, S_FALLING);
		break;

	case STALLED:
		newstate = FLYING;
		goto commonstall;

	case GHOSTSTALLED:
		newstate = GHOST;
		goto commonstall;

	case WOUNDSTALL:
		newstate = WOUNDED;

	      commonstall:
		stalled = ob->ob_angle != (3 * ANGLES / 4)
			|| ob->ob_speed < gminspeed;
		if (!stalled)
			ob->ob_state = state = newstate;
		goto controlled;

	case FLYING:
	case WOUNDED:
	case GHOST:
		stalled = ob->ob_y >= MAX_Y;
		if (stalled) {
			if (playmode == PLAYMODE_NOVICE) {
				ob->ob_angle = (3 * ANGLES / 4);
				stalled = FALSE;
			} else {
				stallpln(ob);
				state = ob->ob_state;
			}
		}

	     controlled:
		if (ob->ob_goingsun)
			break;

		if (ob->ob_life <= 0 && !ob->ob_athome
		    && (state == FLYING || state == STALLED
			|| state == WOUNDED
			|| state == WOUNDSTALL)) {
			hitpln(ob);
			scorepln(ob);
			return movepln(ob);
		}

		if (ob->ob_firing)
			initshot(ob, NULL);

		if (ob->ob_bombing)
			initbomb(ob);

		if (ob->ob_mfiring)
			initmiss(ob);

		if (ob->ob_bfiring)
			initburst(ob);

		nangle = ob->ob_angle;
		nspeed = ob->ob_speed;
		update = ob->ob_flaps;

		if (update) {
			if (ob->ob_orient)
				nangle -= update;
			else
				nangle += update;
			nangle = (nangle + ANGLES) % ANGLES;
		}

		if (!(countmove & 0x0003)) {
			if (!stalled && nspeed < gminspeed
			    && playmode != PLAYMODE_NOVICE) {
				--nspeed;
				update = TRUE;
			} else {
				limit = gminspeed
				    + ob->ob_accel + gravity[nangle];
				if (nspeed < limit) {
					++nspeed;
					update = TRUE;
				} else if (nspeed > limit) {
					--nspeed;
					update = TRUE;
				}
			}
		}

		if (update) {
			if (ob->ob_athome)
				if (ob->ob_accel || ob->ob_flaps)
					nspeed = gminspeed;
				else
					nspeed = 0;

			else if (nspeed <= 0 && !stalled) {
				if (playmode == PLAYMODE_NOVICE)
					nspeed = 1;
				else {
					stallpln(ob);
					return movepln(ob);
				}
			}

			ob->ob_speed = nspeed;
			ob->ob_angle = nangle;

			if (stalled) {
				ob->ob_dx = ob->ob_ldx = ob->ob_ldy = 0;
				ob->ob_dy = -nspeed;
			} else
				setdxdy(ob,
					nspeed * COS(nangle),
					nspeed * SIN(nangle));
		}

		if (stalled) {
			--ob->ob_hitcount;
			if (ob->ob_hitcount <= 0) {
				ob->ob_orient = !ob->ob_orient;
				ob->ob_angle = ((3 * ANGLES / 2)
						- ob->ob_angle)
				    % ANGLES;
				ob->ob_hitcount = STALLCOUNT;
			}
		}

		if (!compplane) {
			ob->ob_life -= ob->ob_speed;
		}

		if (ob->ob_speed)
			ob->ob_athome = FALSE;
		break;
	default:
		break;
	}

	if (ob->ob_endsts == WINNER && ob->ob_goingsun)
		ob->ob_newsym = symbol_plane_win[endcount / 18]; 
	else if (ob->ob_state == FINISHED)
		ob->ob_newsym = NULL;
	else if (ob->ob_state == FALLING && !ob->ob_dx && ob->ob_dy < 0)
		ob->ob_newsym = symbol_plane_hit[ob->ob_orient];
	else
		ob->ob_newsym = symbol_plane[ob->ob_orient][ob->ob_angle];

	//ob->ob_newsym = 
	//ob->ob_state == FINISHED ? NULL :
	//((ob->ob_state == FALLING
	//&& !ob->ob_dx && ob->ob_dy < 0)
	//? swhitsym[ob->ob_orient]
	//: swplnsym[ob->ob_orient][ob->ob_angle]);

	movexy(ob, &x, &y);

	if (x < 0)
		x = ob->ob_x = 0;
	else if (x >= (MAX_X - 16))
		x = ob->ob_x = MAX_X - 16;

	if (!compplane
	    && (ob->ob_state == FLYING
		|| ob->ob_state == STALLED
		|| ob->ob_state == WOUNDED
		|| ob->ob_state == WOUNDSTALL)
	    && consoleplayer->ob_endsts == PLAYING)
		nearpln(ob);

	deletex(ob);
	insertx(ob, ob->ob_xnext);

	if (ob->ob_bdelay)
		--ob->ob_bdelay;
	if (ob->ob_mdelay)
		--ob->ob_mdelay;
	if (ob->ob_bsdelay)
		--ob->ob_bsdelay;

	if (!compplane && ob->ob_athome && ob->ob_state == FLYING)
		refuel(ob);

	if (y < MAX_Y && y >= 0) {
		if (ob->ob_state == FALLING
		    || ob->ob_state == WOUNDED
		    || ob->ob_state == WOUNDSTALL)
			initsmok(ob);
		return plyrplane || ob->ob_state < FINISHED;
	}

	return FALSE;
}



static void adjustfall(OBJECTS * obp)
{
	register OBJECTS *ob;

	ob = obp;
	
	--ob->ob_life;
	if (ob->ob_life <= 0) {
		if (ob->ob_dy < 0) {
			if (ob->ob_dx < 0)
				++ob->ob_dx;
			else if (ob->ob_dx > 0)
				--ob->ob_dx;
		}
		if (ob->ob_dy > -10)
			--ob->ob_dy;
		ob->ob_life = BOMBLIFE;
	}
}


BOOL moveshot(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y;

	ob = obp;
	deletex(ob);
	
	--ob->ob_life;

	if (ob->ob_life <= 0) {
		deallobj(ob);
		return FALSE;
	}

	movexy(ob, &x, &y);

	if (y >= MAX_Y || y <= (int) ground[x]
	    || x < 0 || x >= MAX_X) {
		deallobj(ob);
		return FALSE;
	}

	insertx(ob, ob->ob_xnext);
	ob->ob_newsym = &symbol_pixel;
	return TRUE;
}



BOOL movebomb(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y;

	ob = obp;

	deletex(ob);

	if (ob->ob_life < 0) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return FALSE;
	}

	adjustfall(ob);

	if (ob->ob_dy <= 0)
		initsound(ob, S_BOMB);

	movexy(ob, &x, &y);

	if (y < 0 || x < 0 || x >= MAX_X) {
		deallobj(ob);
		stopsound(ob);
		ob->ob_state = FINISHED;
		return FALSE;
	}

	ob->ob_newsym = symbol_bomb[symangle(ob)]; 
	insertx(ob, ob->ob_xnext);

	if (y >= MAX_Y)
		return FALSE;

	return TRUE;
}



BOOL movemiss(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y, angle;
	OBJECTS *obt;

	ob = obp;

	deletex(ob);

	if (ob->ob_life < 0) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return FALSE;
	}

	if (ob->ob_state == FLYING) {
		obt = ob->ob_target;

		if (obt != ob->ob_owner && (ob->ob_life & 1)) {
			if (obt->ob_target)
				obt = obt->ob_target;
			aim(ob, obt->ob_x, obt->ob_y, NULL, NO);
			angle = ob->ob_angle
			    =
			    (ob->ob_angle + ob->ob_flaps +
			     ANGLES) % ANGLES;
			setdxdy(ob, 
				ob->ob_speed * COS(angle),
				ob->ob_speed * SIN(angle));
		}
		movexy(ob, &x, &y);

		--ob->ob_life;

		if (ob->ob_life <= 0 || y >= ((MAX_Y * 3) / 2)) {
			ob->ob_state = FALLING;
			++ob->ob_life;
		}
	} else {
		adjustfall(ob);
		ob->ob_angle = (ob->ob_angle + 1) % ANGLES;
		movexy(ob, &x, &y);
	}

	if (y < 0 || x < 0 || x >= MAX_X) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return FALSE;
	}

	ob->ob_newsym = symbol_missile[ob->ob_angle];
	insertx(ob, ob->ob_xnext);

	if (y >= MAX_Y)
		return FALSE;

	return TRUE;
}



BOOL moveburst(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y;

	ob = obp;
	deletex(ob);
	if (ob->ob_life < 0) {
		ob->ob_owner->ob_target = NULL;
		deallobj(ob);
		return FALSE;
	}

	adjustfall(ob);
	movexy(ob, &x, &y);

	if (y <= (int) ground[x] || x < 0 || x >= MAX_X) {
		ob->ob_owner->ob_target = NULL;
		deallobj(ob);
		return FALSE;
	}

	ob->ob_owner->ob_target = ob;
	ob->ob_newsym = symbol_burst[ob->ob_life & 1]; 
	insertx(ob, ob->ob_xnext);

	return y < MAX_Y;
}




BOOL movetarg(OBJECTS * obt)
{
	int r;
	register OBJECTS *obp, *ob;

	ob = obt;
	obp = objtop;
	ob->ob_firing = NULL;
	if (gamenum 
	    && ob->ob_state == STANDING
	    && (obp->ob_state == FLYING
		|| obp->ob_state == STALLED
		|| obp->ob_state == WOUNDED
		|| obp->ob_state == WOUNDSTALL)
	    && ob->ob_clr != obp->ob_clr
	    && (gamenum > 1 || (countmove & 0x0001))
	    && ((r = range(ob->ob_x, ob->ob_y, obp->ob_x, obp->ob_y)) > 0)
	    && r < targrnge)
		initshot(ob, ob->ob_firing = obp);

	--ob->ob_hitcount;

	if (ob->ob_hitcount < 0)
		ob->ob_hitcount = 0;

	if (ob->ob_state == STANDING) 
		ob->ob_newsym = symbol_targets[ob->ob_orient];
	else
		ob->ob_newsym = symbol_target_hit;

	return TRUE;
}



BOOL moveexpl(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y;
	register int orient;

	ob = obp;
	orient = ob->ob_orient;
	deletex(ob);
	if (ob->ob_life < 0) {
		if (orient)
			stopsound(ob);
		deallobj(ob);
		return FALSE;
	}

	--ob->ob_life;

	if (ob->ob_life <= 0) {
		if (ob->ob_dy < 0) {
			if (ob->ob_dx < 0)
				++ob->ob_dx;
			else if (ob->ob_dx > 0)
				--ob->ob_dx;
		}
		if ((ob->ob_orient && ob->ob_dy > -10)
		    || (!ob->ob_orient && ob->ob_dy > -gminspeed))
			--ob->ob_dy;
		ob->ob_life = EXPLLIFE;
	}

	movexy(ob, &x, &y);

	if (y <= (int) ground[x]
	    || x < 0 || x >= MAX_X) {
		if (orient)
			stopsound(ob);
		deallobj(ob);
		return (FALSE);
	}
	++ob->ob_hitcount;

	insertx(ob, ob->ob_xnext);
	ob->ob_newsym = symbol_debris[ob->ob_orient];

	return y < MAX_Y;
}



BOOL movesmok(OBJECTS * obp)
{
	register OBJECTS *ob;
	register obstate_t state;

	ob = obp;

	state = ob->ob_owner->ob_state;

	--ob->ob_life;

	if (ob->ob_life <= 0
	    || (state != FALLING
		&& state != WOUNDED
		&& state != WOUNDSTALL
		&& state != CRASHED)) {
		deallobj(ob);
		return FALSE;
	}
	ob->ob_newsym = &symbol_pixel;

	return TRUE;
}



BOOL moveflck(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y;

	ob = obp;
	deletex(ob);

	if (ob->ob_life == -1) {
		deallobj(ob);
		return FALSE;
	}

	--ob->ob_life;

	if (ob->ob_life <= 0) {
		ob->ob_orient = !ob->ob_orient;
		ob->ob_life = FLOCKLIFE;
	}

	if (ob->ob_x < MINFLCKX || ob->ob_x > MAXFLCKX)
		ob->ob_dx = -ob->ob_dx;

	movexy(ob, &x, &y);
	insertx(ob, ob->ob_xnext);
	ob->ob_newsym = symbol_flock[ob->ob_orient]; 
	return TRUE;
}



BOOL movebird(OBJECTS * obp)
{
	register OBJECTS *ob;
	int x, y;

	ob = obp;

	deletex(ob);

	if (ob->ob_life == -1) {
		deallobj(ob);
		return FALSE;
	} else if (ob->ob_life == -2) {
		ob->ob_dy = -ob->ob_dy;
		ob->ob_dx = (countmove & 7) - 4;
		ob->ob_life = BIRDLIFE;
	} else { 
		--ob->ob_life;
		
		if (ob->ob_life <= 0) {
			ob->ob_orient = !ob->ob_orient;
			ob->ob_life = BIRDLIFE;
		}
	}

	movexy(ob, &x, &y);

	insertx(ob, ob->ob_xnext);
	ob->ob_newsym = symbol_bird[ob->ob_orient];
	if (y >= MAX_Y || y <= (int) ground[x]
	    || x < 0 || x >= MAX_X) {
		ob->ob_y -= ob->ob_dy;
		ob->ob_life = -2;
		return FALSE;
	}
	return TRUE;
}




BOOL moveox(OBJECTS * ob)
{
	ob->ob_newsym = symbol_ox[ob->ob_state != STANDING];
	return TRUE;
}




BOOL crashpln(OBJECTS * obp)
{
	register OBJECTS *ob, *obo;

	ob = obp;

	if (ob->ob_dx < 0)
		ob->ob_angle = (ob->ob_angle + 2) % ANGLES;
	else
		ob->ob_angle = (ob->ob_angle + ANGLES - 2) % ANGLES;

	ob->ob_state = ob->ob_state >= GHOST ? GHOSTCRASHED : CRASHED;
	ob->ob_athome = FALSE;
	ob->ob_dx = ob->ob_dy = ob->ob_ldx = ob->ob_ldy = ob->ob_speed = 0;

	obo = &oobjects[ob->ob_index];
	ob->ob_hitcount = ((abs(obo->ob_x - ob->ob_x) < SAFERESET)
			   && (abs(obo->ob_y - ob->ob_y) < SAFERESET))
	    ? (MAXCRCOUNT << 1) : MAXCRCOUNT;

	return TRUE;
}



BOOL hitpln(OBJECTS * obp)
{
	register OBJECTS *ob;

	ob = obp;
	ob->ob_ldx = ob->ob_ldy = 0;
	ob->ob_hitcount = FALLCOUNT;
	ob->ob_state = FALLING;
	ob->ob_athome = FALSE;

	return TRUE;
}




BOOL insertx(OBJECTS * ob, OBJECTS * obp)
{
	register OBJECTS *obs;
	register int obx;

	obs = obp;
	obx = ob->ob_x;
	if (obx < obs->ob_x)
		do {
			obs = obs->ob_xprev;
		} while (obx < obs->ob_x);
	else {
		while (obx >= obs->ob_x)
			obs = obs->ob_xnext;
		obs = obs->ob_xprev;
	}
	ob->ob_xnext = obs->ob_xnext;
	ob->ob_xprev = obs;
	obs->ob_xnext->ob_xprev = ob;
	obs->ob_xnext = ob;

	return TRUE;
}



void deletex(OBJECTS * obp)
{
	register OBJECTS *ob;

	ob = obp;
	ob->ob_xnext->ob_xprev = ob->ob_xprev;
	ob->ob_xprev->ob_xnext = ob->ob_xnext;
}



//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.15  2004/10/25 19:58:06  fraggle
// Remove 'goingsun' global variable
//
// Revision 1.14  2004/10/20 19:00:01  fraggle
// Remove currobx, endsts variables
//
// Revision 1.13  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.12  2004/10/15 18:51:24  fraggle
// Fix the map. Rename dispworld to dispmap as this is what it really does.
//
// Revision 1.11  2004/10/15 16:39:32  fraggle
// Unobfuscate some parts
//
// Revision 1.10  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.9  2003/06/08 03:41:42  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.8  2003/06/08 02:48:45  fraggle
// Remove dispdx, always calculated displx from the current player position
// and do proper edge-of-level bounds checking
//
// Revision 1.7  2003/06/08 02:39:25  fraggle
// Initial code to remove XOR based drawing
//
// Revision 1.6.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.6  2003/06/04 17:13:26  fraggle
// Remove disprx, as it is implied from displx anyway.
//
// Revision 1.5  2003/06/04 17:02:37  fraggle
// Remove some obfuscation and dead code
//
// Revision 1.4  2003/04/06 22:01:02  fraggle
// Fix compile warnings
//
// Revision 1.3  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.2  2003/04/05 22:31:29  fraggle
// Remove PLAYMODE_MULTIPLE and swnetio.c
//
// Revision 1.1.1.1  2003/02/14 19:03:15  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
//                autohome on harry keys mode fixed.
// sdh 27/06/2002: move to new sopsym_t for symbols
// sdh 26/03/2002: change CGA_ to Vid_
// sdh 27/10/2001: fix refueling i broke with the guages change yesterday
// sdh 26/10/2001: use new dispguages function
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: reformatted with indent. edited some code by hand to
//                 make it more readable
// sdh 19/10/2001: removed all externs, these are now in headers
//                 shuffled some functions around to shut up compiler
// sdh 18/10/2001: converted all functions in this file to ANSI-style arguments
//
// 87-04-09        Delay between starbursts.
// 87-04-04        Missile and starburst support.
// 87-04-01        Missiles.
// 87-03-31        Allow wounded plane to fly home
// 87-03-30        Novice Player
// 87-03-12        Computer plane heads home at end.
// 87-03-12        Prioritize bombs/shots over flaps.
// 87-03-12        Proper ASYCHRONOUS end of game.
// 87-03-12        Crashed planes stay longer at home.
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 85-10-31        Atari
// 84-02-07        Development
//
//---------------------------------------------------------------------------


