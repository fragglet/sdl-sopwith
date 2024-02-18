//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
//        swmove   -      SW move all objects and players
//

#include "sw.h"
#include "swauto.h"
#include "swcollsn.h"
#include "swdisp.h"
#include "swend.h"
#include "swinit.h"
#include "swmain.h"
#include "swmove.h"
#include "swobject.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swtitle.h"

// If the player manages to keep the plane in the air this long, they
// have got the hang of the controls and don't need any more help.
#define SUCCESSFUL_FLIGHT_TIME (8 /* seconds */ * FPS)

static bool movepln(OBJECTS *ob);
static void interpret(OBJECTS *ob, int key);

static bool quit;
static int last_ground_time = 0;
bool successful_flight = false;

void swmove(void)
{
	OBJECTS *ob, *obn;

	if (deltop) {
		delbot->ob_next = objfree;
		objfree = deltop;
		deltop = delbot = NULL;
	}

	++dispcnt;

	if (dispcnt >= keydelay) {
		dispcnt = 0;
	}

	ob = objtop;
	while (ob) {
		obn = ob->ob_next;
		ob->ob_drwflg = (*ob->ob_movef) (ob);
		ob = obn;
	}

	++countmove;
	if (consoleplayer->ob_athome) {
		last_ground_time = countmove;
	} else if (countmove - last_ground_time > SUCCESSFUL_FLIGHT_TIME) {
		successful_flight = true;
	}
}


static void nearpln(OBJECTS *ob)
{
	OBJECTS *obt, *obc;
	int obx, obclr;

	obx = ob->ob_x;
	obclr = ob->ob_owner->ob_clr;

	for (obt = objtop; obt != NULL; obt = obt->ob_next) {
		if (obt->ob_type != PLANE
		 || obclr == obt->ob_owner->ob_clr) {
			continue;
		}

		if (obt->ob_drawf == dispcomp) {
			if (playmode != PLAYMODE_COMPUTER
			 || (obx >= obt->ob_original_ob->territory_l
			  && obx <= obt->ob_original_ob->territory_r)) {
				obc = obt->ob_target;
				if (!obc || abs(obx - obt->ob_x)
				          < abs(obc->ob_x - obt->ob_x)) {
					obt->ob_target = ob;
				}
			}
		}
	}
}




static void topup(int *counter, int max)
{
	if (*counter == max) {
		return;
	}
	if (max < 20) {
		if (!(countmove % 20)) {
			++*counter;
		}
	} else {
		*counter += max / 100;
	}
	if (*counter > max) {
		*counter = max;
	}
}


static void refuel(OBJECTS *ob)
{
	// sdh 26/10/2001: top up stuff, if anything happens update
	// the gauges (now a single function)

	topup(&ob->ob_life, MAXFUEL);
	topup(&ob->ob_rounds, MAXROUNDS);
	topup(&ob->ob_bombs, MAXBOMBS);
	topup(&ob->ob_missiles, MAXMISSILES);
	topup(&ob->ob_bursts, MAXBURSTS);
}



static int symangle(OBJECTS * ob)
{
	int dx, dy;

	dx = ob->ob_dx;
	dy = ob->ob_dy;
	if (dx == 0) {
		if (dy < 0) {
			return 6;
		} else if (dy > 0) {
			return 2;
		} else {
			return 6;
		}
	} else if (dx > 0) {
		if (dy < 0) {
			return 7;
		} else if (dy > 0) {
			return 1;
		} else {
			return 0;
		}
	} else if (dy < 0) {
		return 5;
	} else if (dy > 0) {
		return 3;
	} else {
		return 4;
	}
}

bool moveplyr(OBJECTS *ob)
{
	int multkey;

	compplane = false;
	plyrplane = player == ob->ob_plrnum;

	endstat = consoleplayer->ob_endsts;

	if (endstat) {
		--endcount;
		if (endcount <= 0) {
			if (playmode != PLAYMODE_ASYNCH && !quit) {
				swrestart();
				return true;
			}
			swend(NULL, true);
		}
	}
	
	// get move command for this tic
	
	multkey = latest_player_commands[ob->ob_plrnum][countmove % MAX_NET_LAG];

	// Thanks to Kodath duMatri for fixing this :)

	if ((multkey & K_HARRYKEYS) != 0 && ob->ob_orient) {
		if (multkey & (K_FLAPU | K_FLAPD)) {
			multkey ^= K_FLAPU | K_FLAPD;
		}
	}

	interpret(ob, multkey);

	/*
	if (dispcnt) {
		ob->ob_flaps = 0;
		ob->ob_bfiring = ob->ob_bombing = false;
		ob->ob_mfiring = NULL;
	}*/

	if (ob->ob_state == CRASHED && ob->ob_hitcount <= 0) {

		// sdh: infinite lives in multiplayer mode

		if (playmode != PLAYMODE_ASYNCH) {
			++ob->ob_crashcnt;
		}

		if (endstat != WINNER
		 && (ob->ob_life <= QUIT
		  || (playmode != PLAYMODE_ASYNCH
		   && ob->ob_crashcnt >= MAXCRASH))) {
			if (!endstat) {
				loser(ob);
			}
		} else {
			initplyr(ob);
			initdisp(true);
			if (endstat == WINNER) {
				if (ctlbreak()) {
					swend(NULL, true);
				}
				winner(ob);
			}
		}
	}

	return movepln(ob);
}




static void interpret(OBJECTS *ob, int key)
{
	obstate_t state;

	ob->ob_flaps = 0;
	ob->ob_bombing = ob->ob_bfiring = 0;
	ob->ob_mfiring = ob->ob_firing = NULL;

	state = ob->ob_state;

	if (state != FLYING && state != STALLED && state != FALLING
	 && state != WOUNDED && state != WOUNDSTALL) {
		return;
	}

	if (state != FALLING) {
		if (endstat) {
			if (endstat == LOSER && plyrplane) {
				gohome(ob);
			}
			return;
		}

		if (key & K_BREAK) {
			ob->ob_life = QUIT;
			ob->ob_home = false;
			if (ob->ob_athome) {
				ob->ob_state = state = CRASHED;
				ob->ob_hitcount = 0;
			}
			if (plyrplane) {
				quit = true;
			}
		}

		if (key & K_HOME) {
			if (state == FLYING || state == WOUNDED) {
				ob->ob_home = true;
			}
		}
	}

	if ((countmove & 1)
	 || (state != WOUNDED && state != WOUNDSTALL)) {
		if (key & K_FLAPU) {
			++ob->ob_flaps;
			ob->ob_home = false;
		}

		if (key & K_FLAPD) {
			--ob->ob_flaps;
			ob->ob_home = false;
		}

		// We don't allow flipping upside down while sitting
		// on the runway, that would be silly (this was a bug
		// in the original game).
		if ((key & K_FLIP) && !ob->ob_athome) {
			ob->ob_orient = !ob->ob_orient;
			ob->ob_home = false;
		}

		if (key & K_DEACC) {
			if (ob->ob_accel) {
				--ob->ob_accel;
			}
			ob->ob_home = false;
		}

		if (key & K_ACCEL) {
			if (ob->ob_accel < MAX_THROTTLE) {
				++ob->ob_accel;
			}
			ob->ob_home = false;
		}
	}

	if ((key & K_SHOT) && state < FINISHED) {
		ob->ob_firing = ob;
	}

	if ((key & K_MISSILE) && state < FINISHED) {
		ob->ob_mfiring = ob;
	}

	if ((key & K_BOMB) && state < FINISHED) {
		ob->ob_bombing = true;
	}

	if ((key & K_STARBURST) && state < FINISHED) {
		ob->ob_bfiring = true;
	}

	if (key & K_SOUND) {
		if (plyrplane) {
			if (soundflg) {
				sound(0, 0, NULL);
				swsound();
			}
			soundflg = !soundflg;
		}
	}

	if (ob->ob_home) {
		gohome(ob);
	}
}

bool movecomp(OBJECTS *ob)
{
	int rc;

	compplane = true;
	plyrplane = false;

	ob->ob_flaps = 0;
	ob->ob_bfiring = ob->ob_bombing = false;
	ob->ob_mfiring = NULL;

	endstat = ob->ob_endsts;

	if (!dispcnt) {
		ob->ob_firing = NULL;
	}

	switch (ob->ob_state) {

	case WOUNDED:
	case WOUNDSTALL:
		if (countmove & 1) {
			break;
		}

	case FLYING:
	case STALLED:
		if (endstat) {
			gohome(ob);
			break;
		}
		if (!dispcnt) {
			swauto(ob);
		}
		break;

	case CRASHED:
		ob->ob_firing = NULL;
		if (ob->ob_hitcount <= 0 && !endstat) {
			initcomp(ob);
		}
		break;

	default:
		ob->ob_firing = NULL;
		break;
	}


	rc = movepln(ob);

	return rc;
}

static bool stallpln(OBJECTS *ob)
{
	ob->ob_ldx = ob->ob_ldy = ob->ob_orient = ob->ob_dx = 0;
	ob->ob_angle = 7 * ANGLES / 8;
	ob->ob_speed = 0;
	ob->ob_dy = 0;
	ob->ob_hitcount = STALLCOUNT;
	ob->ob_state =
		ob->ob_state == WOUNDED ? WOUNDSTALL : STALLED;
	ob->ob_athome = false;

	return true;
}



static bool movepln(OBJECTS *ob)
{
	int nangle, nspeed, limit, update;
	obstate_t state, newstate;
	int x, y, stalled;

	static signed int gravity[] = {
		0, -1, -2, -3, -4, -3, -2, -1,
		0, 1, 2, 3, 4, 3, 2, 1
	};

	state = ob->ob_state;

	switch (state) {
	case FINISHED:
	case WAITING:
		return false;

	case CRASHED:
		--ob->ob_hitcount;
		break;

	case FALLING:
		ob->ob_hitcount -= 2;
		if ((ob->ob_dy < 0) && ob->ob_dx) {
			if (ob->ob_orient ^ (ob->ob_dx < 0)) {
				ob->ob_hitcount -= ob->ob_flaps;
			} else {
				ob->ob_hitcount += ob->ob_flaps;
			}
		}

		if (ob->ob_hitcount <= 0) {
			if (ob->ob_dy < 0) {
				if (ob->ob_dx < 0) {
					++ob->ob_dx;
				} else if (ob->ob_dx > 0) {
					--ob->ob_dx;
				} else {
					ob->ob_orient = !ob->ob_orient;
				}
			}

			if (ob->ob_dy > -10) {
				--ob->ob_dy;
			}
			ob->ob_hitcount = FALLCOUNT;
		}
		ob->ob_angle = symangle(ob) * 2;
		if (ob->ob_dy <= 0) {
			initsound(ob, S_FALLING);
		}
		break;

	case STALLED:
		newstate = FLYING;
		goto commonstall;

	case WOUNDSTALL:
		newstate = WOUNDED;

	      commonstall:
		stalled = ob->ob_angle != (3 * ANGLES / 4)
			|| ob->ob_speed < gminspeed;
		if (!stalled) {
			ob->ob_state = state = newstate;
		}
		goto controlled;

	case FLYING:
	case WOUNDED:
		stalled = ob->ob_y >= MAX_Y;
		if (stalled) {
			if (playmode == PLAYMODE_NOVICE) {
				ob->ob_angle = (3 * ANGLES / 4);
				stalled = false;
			} else {
				stallpln(ob);
				state = ob->ob_state;
			}
		}

	     controlled:
		if (ob->ob_goingsun) {
			break;
		}

		if (ob->ob_life <= 0 && !ob->ob_athome
		 && (state == FLYING || state == STALLED
		  || state == WOUNDED || state == WOUNDSTALL)) {
			hitpln(ob);
			scorepln(ob, GROUND);
			return movepln(ob);
		}

		if (ob->ob_firing) {
			initshot(ob, NULL);
		}

		if (ob->ob_bombing) {
			initbomb(ob);
		}

		if (ob->ob_mfiring) {
			initmiss(ob);
		}

		if (ob->ob_bfiring) {
			initburst(ob);
		}

		nangle = ob->ob_angle;
		nspeed = ob->ob_speed;
		update = ob->ob_flaps;

		if (update) {
			if (ob->ob_orient) {
				nangle -= update;
			} else {
				nangle += update;
			}
			nangle = (nangle + ANGLES) % ANGLES;
		}

		if (!(countmove & 0x0003)) {
			if (!stalled && nspeed < gminspeed
			 && playmode != PLAYMODE_NOVICE) {
				--nspeed;
				update = true;
			} else {
				limit = gminspeed
				      + ob->ob_accel + gravity[nangle];
				if (nspeed < limit) {
					++nspeed;
					update = true;
				} else if (nspeed > limit) {
					--nspeed;
					update = true;
				}
			}
		}

		if (update) {
			if (ob->ob_athome) {
				if (ob->ob_accel || ob->ob_flaps) {
					nspeed = gminspeed;
				} else {
					nspeed = 0;
				}

			} else if (nspeed <= 0 && !stalled) {
				if (playmode == PLAYMODE_NOVICE) {
					nspeed = 1;
				} else {
					stallpln(ob);
					return movepln(ob);
				}
			}

			ob->ob_speed = nspeed;
			ob->ob_angle = nangle;

			if (stalled) {
				ob->ob_dx = ob->ob_ldx = ob->ob_ldy = 0;
				ob->ob_dy = -nspeed;
			} else {
				setdxdy(ob,
				        nspeed * COS(nangle),
				        nspeed * SIN(nangle));
			}
		}

		if (stalled) {
			--ob->ob_hitcount;
			if (ob->ob_hitcount <= 0) {
				ob->ob_orient = !ob->ob_orient;
				ob->ob_angle =
				    ((3 * ANGLES / 2) - ob->ob_angle)
				    % ANGLES;
				ob->ob_hitcount = STALLCOUNT;
			}
		}

		if (!compplane) {
			ob->ob_life -= ob->ob_speed;
		} else if (ob->ob_life > 100) { /* Just for statistics */
			ob->ob_life -= ob->ob_speed;
		}

		if (ob->ob_speed) {
			ob->ob_athome = false;
		}
		break;
	default:
		break;
	}

	if (ob->ob_endsts == WINNER && ob->ob_goingsun) {
		ob->ob_newsym = &symbol_plane_win[endcount / 18]->sym[0];
	} else if (ob->ob_state == FINISHED) {
		ob->ob_newsym = NULL;
	} else if (ob->ob_state == FALLING && !ob->ob_dx && ob->ob_dy < 0) {
		ob->ob_newsym = &symbol_plane_hit[ob->ob_orient]->sym[0];
	} else if (ob->ob_orient) {
		// Flipped:
		int a = (16 - ob->ob_angle) % 16;
		ob->ob_newsym = &symbol_plane[a % 4]->sym[4 + a / 4];
	} else {
		ob->ob_newsym = &symbol_plane[ob->ob_angle % 4]
			->sym[ob->ob_angle / 4];
	}

	//ob->ob_newsym =
	//ob->ob_state == FINISHED ? NULL :
	//((ob->ob_state == FALLING
	//&& !ob->ob_dx && ob->ob_dy < 0)
	//? swhitsym[ob->ob_orient]
	//: swplnsym[ob->ob_orient][ob->ob_angle]);

	movexy(ob, &x, &y);

	if (x < 0) {
		x = ob->ob_x = 0;
		updateobjpos(ob);
	} else if (x >= (currgame->gm_max_x - 16)) {
		x = ob->ob_x = currgame->gm_max_x - 16;
		updateobjpos(ob);
	}

	if (!compplane
	 && consoleplayer->ob_endsts == PLAYING
	 && (ob->ob_state == FLYING || ob->ob_state == STALLED
	  || ob->ob_state == WOUNDED || ob->ob_state == WOUNDSTALL)) {
		nearpln(ob);
	}

	if (ob->ob_bdelay) {
		--ob->ob_bdelay;
	}
	if (ob->ob_mdelay) {
		--ob->ob_mdelay;
	}
	if (ob->ob_bsdelay) {
		--ob->ob_bsdelay;
	}

	if (!compplane && ob->ob_athome && ob->ob_state == FLYING) {
		refuel(ob);
	}

	if (y < MAX_Y && y >= 0) {
		if (ob->ob_state == FALLING
		 || ob->ob_state == WOUNDED
		 || ob->ob_state == WOUNDSTALL) {
			initsmok(ob);
		}
		return plyrplane || ob->ob_state < FINISHED;
	}

	return false;
}



static void adjustfall(OBJECTS *ob)
{
	--ob->ob_life;
	if (ob->ob_life <= 0) {
		if (ob->ob_dy < 0) {
			if (ob->ob_dx < 0) {
				++ob->ob_dx;
			} else if (ob->ob_dx > 0) {
				--ob->ob_dx;
			}
		}
		if (ob->ob_dy > -10) {
			--ob->ob_dy;
		}
		ob->ob_life = BOMBLIFE;
	}
}


bool moveshot(OBJECTS *ob)
{
	int x, y;

	--ob->ob_life;

	if (ob->ob_life <= 0) {
		deallobj(ob);
		return false;
	}

	movexy(ob, &x, &y);

	if (y >= MAX_Y || x < 0 || x >= currgame->gm_max_x
	 || y <= (int) ground[x]) {
		deallobj(ob);
		return false;
	}

	ob->ob_newsym = &symbol_pixel;
	return true;
}



bool movebomb(OBJECTS *ob)
{
	int x, y;
	int ang;

	if (ob->ob_life < 0) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	adjustfall(ob);

	if (ob->ob_dy <= 0) {
		initsound(ob, S_BOMB);
	}

	movexy(ob, &x, &y);

	if (y < 0 || x < 0 || x >= currgame->gm_max_x) {
		deallobj(ob);
		stopsound(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	ang = symangle(ob);
	ob->ob_newsym = &symbol_bomb[ang % 2]->sym[ang / 2];

	if (y >= MAX_Y) {
		return false;
	}

	return true;
}



bool movemiss(OBJECTS *ob)
{
	int x, y, angle;
	OBJECTS *obt;

	if (ob->ob_life < 0) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	if (ob->ob_state == FLYING) {
		obt = ob->ob_missiletarget;

		if (obt != ob->ob_owner && (ob->ob_life & 1)) {
			if (obt->ob_missiletarget) {
				obt = obt->ob_missiletarget;
			}
			aim(ob, obt->ob_x, obt->ob_y, NULL, false);
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

	if (y < 0 || x < 0 || x >= currgame->gm_max_x) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	ob->ob_newsym =
		&symbol_missile[ob->ob_angle % 4]->sym[ob->ob_angle / 4];

	if (y >= MAX_Y) {
		return false;
	}

	return true;
}



bool moveburst(OBJECTS *ob)
{
	int x, y;

	if (ob->ob_life < 0) {
		ob->ob_owner->ob_missiletarget = NULL;
		deallobj(ob);
		return false;
	}

	adjustfall(ob);
	movexy(ob, &x, &y);

	if (x < 0 || x >= currgame->gm_max_x || y <= (int) ground[x]) {
		ob->ob_owner->ob_missiletarget = NULL;
		deallobj(ob);
		return false;
	}

	ob->ob_owner->ob_missiletarget = ob;
	ob->ob_newsym = &symbol_burst[ob->ob_life & 1]->sym[0];

	return y < MAX_Y;
}




bool movetarg(OBJECTS *ob)
{
	int r;
	OBJECTS *obp;

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
	 && r < targrnge) {
		initshot(ob, ob->ob_firing = obp);
	}

	--ob->ob_hitcount;

	if (ob->ob_hitcount < 0) {
		ob->ob_hitcount = 0;
	}

	if (ob->ob_state == STANDING) {
		ob->ob_newsym = &symbol_targets[ob->ob_orient]->sym[0];
	} else {
		ob->ob_newsym = &symbol_target_hit->sym[0];
	}

	return true;
}



bool moveexpl(OBJECTS * obp)
{
	OBJECTS *ob;
	int x, y;
	int orient;

	ob = obp;
	orient = ob->ob_orient;
	if (ob->ob_life < 0) {
		if (orient) {
			stopsound(ob);
		}
		deallobj(ob);
		return false;
	}

	--ob->ob_life;

	if (ob->ob_life <= 0) {
		if (ob->ob_dy < 0) {
			if (ob->ob_dx < 0) {
				++ob->ob_dx;
			} else if (ob->ob_dx > 0) {
				--ob->ob_dx;
			}
		}
		if ((ob->ob_orient && ob->ob_dy > -10)
		 || (!ob->ob_orient && ob->ob_dy > -gminspeed)) {
			--ob->ob_dy;
		}
		ob->ob_life = EXPLLIFE;
	}

	movexy(ob, &x, &y);

	if (x < 0 || x >= currgame->gm_max_x || y <= (int) ground[x]) {
		if (orient) {
			stopsound(ob);
		}
		deallobj(ob);
		return (false);
	}
	++ob->ob_hitcount;

	ob->ob_newsym = &symbol_debris[ob->ob_orient]->sym[0];

	return y < MAX_Y;
}



bool movesmok(OBJECTS * obp)
{
	OBJECTS *ob;
	obstate_t state;

	ob = obp;

	state = ob->ob_owner->ob_state;

	--ob->ob_life;

	if (ob->ob_life <= 0
	 || (state != FALLING && state != WOUNDED
	  && state != WOUNDSTALL && state != CRASHED)) {
		deallobj(ob);
		return false;
	}
	ob->ob_newsym = &symbol_pixel;

	return true;
}



bool moveflck(OBJECTS * obp)
{
	OBJECTS *ob;
	int x, y;

	ob = obp;

	if (ob->ob_life == -1) {
		deallobj(ob);
		return false;
	}

	--ob->ob_life;

	if (ob->ob_life <= 0) {
		ob->ob_orient = !ob->ob_orient;
		ob->ob_life = FLOCKLIFE;
	}

	// Flocks fly back and forth within their "territory".
	if (ob->ob_x < ob->ob_original_ob->territory_l) {
		ob->ob_dx = abs(ob->ob_dx);
	} else if (ob->ob_x > ob->ob_original_ob->territory_r) {
		ob->ob_dx = -abs(ob->ob_dx);
	}

	movexy(ob, &x, &y);
	ob->ob_newsym = &symbol_flock[ob->ob_orient]->sym[0];
	return true;
}


static bool checkwall(OBJECTS *obp, int direction)
{
	int check_x, cnt;

	check_x = obp->ob_x;
	for (cnt = 0; cnt < 20; ++cnt) {
		if (check_x < 0 || check_x >= currgame->gm_max_x) {
			return true;
		}
		if ((int) ground[check_x] > obp->ob_y + 10) {
			return true;
		}
		if (direction < 0) {
			--check_x;
		} else {
			++check_x;
		}
	}
	return false;
}

bool movebird(OBJECTS * obp)
{
	OBJECTS *ob;
	int x, y;

	ob = obp;

	if (ob->ob_life == -1) {
		deallobj(ob);
		return false;
	} else if (ob->ob_life == -2) {
		ob->ob_dy = -ob->ob_dy;
		ob->ob_dx = (countmove & 7) - 4;
		// Don't move in a direction where we might (continue to?)
		// fly into a wall. Fixes a crasher bug.
		if (checkwall(ob, ob->ob_dx)) {
			ob->ob_dx = -ob->ob_dx;
		}
		ob->ob_life = BIRDLIFE;
	} else {
		--ob->ob_life;
		
		if (ob->ob_life <= 0) {
			ob->ob_orient = !ob->ob_orient;
			ob->ob_life = BIRDLIFE;
		}
	}

	movexy(ob, &x, &y);

	ob->ob_newsym = &symbol_bird[ob->ob_orient]->sym[0];
	if (x < 0 || x >= currgame->gm_max_x
	 || y >= MAX_Y || y <= (int) ground[x]) {
		ob->ob_y -= ob->ob_dy;
		ob->ob_life = -2;
		return false;
	}
	return true;
}




bool moveox(OBJECTS * ob)
{
	ob->ob_newsym = &symbol_ox[ob->ob_state != STANDING]->sym[0];
	return true;
}




bool crashpln(OBJECTS *ob)
{
	const original_ob_t *orig_ob = ob->ob_original_ob;

	if (ob->ob_dx < 0) {
		ob->ob_angle = (ob->ob_angle + 2) % ANGLES;
	} else {
		ob->ob_angle = (ob->ob_angle + ANGLES - 2) % ANGLES;
	}

	ob->ob_state = CRASHED;
	ob->ob_athome = false;
	ob->ob_dx = ob->ob_dy = ob->ob_ldx = ob->ob_ldy = ob->ob_speed = 0;

	ob->ob_hitcount = ((abs(orig_ob->x - ob->ob_x) < SAFERESET)
	                && (abs(ob->ob_orig_y - ob->ob_y) < SAFERESET))
	    ? (MAXCRCOUNT << 1) : MAXCRCOUNT;

	return true;
}



bool hitpln(OBJECTS * obp)
{
	OBJECTS *ob;

	ob = obp;
	ob->ob_ldx = ob->ob_ldy = 0;
	ob->ob_hitcount = FALLCOUNT;
	ob->ob_state = FALLING;
	ob->ob_athome = false;

	return true;
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
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
