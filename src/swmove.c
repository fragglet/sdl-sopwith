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
//        swmove   -      SW move all objects and players
//

#include <assert.h>

#include "sw.h"
#include "swauto.h"
#include "swcollsn.h"
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
	int obx;

	obx = ob->ob_x;

	for (obt = objtop; obt != NULL; obt = obt->ob_next) {
		// TODO: Planes currently target any plane of a different
		// faction. In the future it would be nice to support
		// alliances between factions.
		if (obt->ob_type != PLANE
		 || obt->ob_faction == ob->ob_faction) {
			continue;
		}

		if (obt->ob_movef == movecomp) {
			if (playmode != PLAYMODE_COMPUTER
			 || in_range(obt->ob_original_ob->territory_l, obx,
			             obt->ob_original_ob->territory_r)) {
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
	*counter = clamp_max(*counter, max);
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
	dy = FIXED_IP(ob->ob_ndy);
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

// Sound callback function for planes
static void PlaneSoundCallback(OBJECTS *ob)
{
	if (ob->ob_firing) {
		sound(S_SHOT, 0, ob);
	} else {
		switch (ob->ob_state) {
		case FALLING:
			if (FIXED_IP(ob->ob_ndy) >= 0) {
				sound(S_HIT, 0, ob);
			} else {
				sound(S_FALLING, ob->ob_y, ob);
			}
			break;

		case FLYING:
			sound(S_PLANE, -ob->ob_speed, ob);
			break;

		case STALLED:
		case WOUNDED:
		case WOUNDSTALL:
			sound(S_HIT, 0, ob);
			break;

		default:
			break;
		}
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
			initplyr(ob, ob->ob_original_ob);
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

	if (PlaneIsKilled(state) && state != FALLING) {
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

	if (!PlaneIsWounded(state) || (countmove & 1) != 0) {
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
			initcomp(ob, ob->ob_original_ob);
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
	ob->ob_ldx = ob->ob_orient = ob->ob_dx = 0;
	ob->ob_angle = 7 * ANGLES / 8;
	ob->ob_speed = 0;
	ob->ob_ndy = 0;
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

	static const signed int gravity[] = {
		0, -1, -2, -3, -4, -3, -2, -1,
		0, 1, 2, 3, 4, 3, 2, 1
	};

	state = ob->ob_state;
	ob->ob_soundf = PlaneSoundCallback;

	switch (state) {
	case FINISHED:
	case WAITING:
		return false;

	case CRASHED:
		--ob->ob_hitcount;
		break;

	case FALLING:
		ob->ob_hitcount -= 2;
		if (FIXED_IP(ob->ob_ndy) < 0 && ob->ob_dx) {
			if (ob->ob_orient ^ (ob->ob_dx < 0)) {
				ob->ob_hitcount -= ob->ob_flaps;
			} else {
				ob->ob_hitcount += ob->ob_flaps;
			}
		}

		if (ob->ob_hitcount <= 0) {
			if (FIXED_IP(ob->ob_ndy) < 0) {
				if (ob->ob_dx < 0) {
					++ob->ob_dx;
				} else if (ob->ob_dx > 0) {
					--ob->ob_dx;
				} else {
					++ob->ob_orient;
				}
			}

			if (FIXED_IP(ob->ob_ndy) > -10) {
				ob->ob_ndy -= FIXED_UNIT;
			}
			ob->ob_hitcount = FALLCOUNT;
		}
		ob->ob_angle = symangle(ob) * 2;
		if (ob->ob_ndy <= 0) {
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
		 && !PlaneIsKilled(state)) {
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
				ob->ob_dx = ob->ob_ldx = 0;
				ob->ob_ndy = -nspeed * FIXED_UNIT;
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
		ob->ob_symbol = &symbol_plane_win[endcount / 18].sym[0];
	} else if (ob->ob_state == FINISHED) {
		ob->ob_symbol = NULL;
	} else if (ob->ob_state == FALLING && !ob->ob_dx && ob->ob_ndy < 0) {
		ob->ob_symbol = &symbol_plane_hit[ob->ob_orient % 4].sym[0];
	} else if (ob->ob_orient) {
		// Flipped:
		int a = (16 - ob->ob_angle) % 16;
		ob->ob_symbol = &symbol_plane[a % 4].sym[4 + a / 4];
	} else {
		ob->ob_symbol = &symbol_plane[ob->ob_angle % 4]
			.sym[ob->ob_angle / 4];
	}

	movexy(ob, &x, &y);

	if (!in_range(0, x, currgame->gm_max_x - 16)) {
		x = clamp_range(0, x, currgame->gm_max_x - 16);
		updateobjpos(ob);
	}

	if (!compplane
	 && consoleplayer->ob_endsts == PLAYING
	 && !PlaneIsKilled(ob->ob_state)) {
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

	if (in_range(0, y, MAX_Y - 1)) {
		if (ob->ob_state == FALLING || PlaneIsWounded(ob->ob_state)) {
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
		if (ob->ob_ndy < 0) {
			if (ob->ob_dx < 0) {
				++ob->ob_dx;
			} else if (ob->ob_dx > 0) {
				--ob->ob_dx;
			}
		}
		if (FIXED_IP(ob->ob_ndy) > -10) {
			ob->ob_ndy -= FIXED_UNIT;
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

	if (!in_range(0, x, currgame->gm_max_x - 1)
	 || !in_range((int) ground[x] + 1, y, MAX_Y - 1)) {
		deallobj(ob);
		return false;
	}

	ob->ob_symbol = &symbol_pixel;
	return true;
}

static void BombSoundCallback(OBJECTS *ob)
{
	if (ob->ob_ndy <= 0) {
		sound(S_BOMB, -ob->ob_y, ob);
	}
}

bool movebomb(OBJECTS *ob)
{
	int x, y;
	int ang;

	ob->ob_soundf = BombSoundCallback;

	if (ob->ob_life < 0) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	adjustfall(ob);

	if (ob->ob_ndy <= 0) {
		initsound(ob, S_BOMB);
	}

	movexy(ob, &x, &y);

	if (y < 0 || !in_range(0, x, currgame->gm_max_x - 1)) {
		deallobj(ob);
		stopsound(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	ang = symangle(ob);
	ob->ob_symbol = &symbol_bomb[ang % 2].sym[ang / 2];

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

	if (y < 0 || !in_range(0, x, currgame->gm_max_x - 1)) {
		deallobj(ob);
		ob->ob_state = FINISHED;
		return false;
	}

	ob->ob_symbol =
		&symbol_missile[ob->ob_angle % 4].sym[ob->ob_angle / 4];

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

	if (!in_range(0, x, currgame->gm_max_x - 1) || y <= (int) ground[x]) {
		ob->ob_owner->ob_missiletarget = NULL;
		deallobj(ob);
		return false;
	}

	ob->ob_owner->ob_missiletarget = ob;
	ob->ob_symbol = &symbol_burst[ob->ob_life & 1].sym[0];

	return y < MAX_Y;
}

static void TargetSoundCallback(OBJECTS * ob)
{
	if (ob->ob_firing) {
		sound(S_SHOT, 0, ob);
	}
}

static OBJECTS *FindEnemyPlane(OBJECTS *ob)
{
	OBJECTS *obp;
	int r;

	// TODO: We can do better than scanning the entire object list.
	for (obp = objtop; obp != NULL; obp = obp->ob_next) {
		// TODO: Targets consider any plane of a different faction
		// to be an enemy. In the future we may want to support
		// alliances between factions.
		if (obp->ob_type != PLANE
		 || obp->ob_faction == ob->ob_faction) {
			continue;
		}
		// In single player mode, computer planes do not get targeted.
		if (playmode != PLAYMODE_ASYNCH
		 && obp->ob_faction != FACTION_PLAYER1) {
			continue;
		}
		if (PlaneIsKilled(obp->ob_state)) {
			continue;
		}
		r = range(ob->ob_x, ob->ob_y, obp->ob_x, obp->ob_y);
		if (in_range(1, r, targrnge - 1)) {
			return obp;
		}
	}

	return NULL;
}

// This table determines how often targets fire at enemy planes; there is one
// entry for each target type. An aggression of zero means that type never
// fires; otherwise the lower the value, the more often it fires.
static const int target_aggression[NUM_TARGET_TYPES] = {
	2,  // TARGET_HANGAR
	2,  // TARGET_FACTORY
	2,  // TARGET_OIL_TANK
	2,  // TARGET_TANK
	5,  // TARGET_TRUCK
	5,  // TARGET_TANKER_TRUCK
	0,  // TARGET_FLAG
	0,  // TARGET_TENT
	2,  // TARGET_CUSTOM1
	2,  // TARGET_CUSTOM2
	2,  // TARGET_CUSTOM3
	5,  // TARGET_CUSTOM4
	5,  // TARGET_CUSTOM5
	0,  // TARGET_CUSTOM_PASSIVE1
	0,  // TARGET_CUSTOM_PASSIVE2
	0,  // TARGET_CUSTOM_PASSIVE3
	0,  // TARGET_CUSTOM_PASSIVE4
	0,  // TARGET_CUSTOM_PASSIVE5
	0,  // TARGET_RADIO_TOWER
	0,  // TARGET_WATER_TOWER
};

bool movetarg(OBJECTS *ob)
{
	sopsym_t *oldsym = ob->ob_symbol;
	int transform = ob->ob_original_ob->transform;
	int aggression;

	ob->ob_soundf = TargetSoundCallback;
	ob->ob_firing = NULL;

	assert(ob->ob_orient < arrlen(target_aggression));
	aggression = target_aggression[ob->ob_orient];

	if (ob->ob_state == STANDING
	 && gamenum > 0
	 && aggression > 0
	 && (gamenum > 1 || (countmove % aggression) == (aggression - 1))) {
		OBJECTS *plane = FindEnemyPlane(ob);
		if (plane) {
			initshot(ob, plane);
			ob->ob_firing = plane;
		}
	}

	ob->ob_hitcount = clamp_min(ob->ob_hitcount - 1, 0);

	if (ob->ob_state == STANDING) {
		ob->ob_symbol =
			&symbol_targets[ob->ob_orient].sym[transform];
	} else {
		ob->ob_symbol =
			&symbol_target_hit[ob->ob_orient].sym[transform];
	}

	// Symbol changes on explosion, and the new sprite might be a
	// different size. Stay centered, and if the symbol height changes, we
	// need to adjust Y so the target stays on the ground.
	ob->ob_x += (oldsym->w - ob->ob_symbol->w) / 2;
	ob->ob_y -= oldsym->h - ob->ob_symbol->h;

	return true;
}

static void ExplosionSoundCallback(OBJECTS *ob)
{
	if (ob->ob_orient) {
		sound(S_EXPLOSION, ob->ob_hitcount, ob);
	}
}

bool moveexpl(OBJECTS * obp)
{
	OBJECTS *ob;
	int x, y;
	int orient;

	obp->ob_soundf = ExplosionSoundCallback;

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
		if (ob->ob_ndy < 0) {
			if (ob->ob_dx < 0) {
				++ob->ob_dx;
			} else if (ob->ob_dx > 0) {
				--ob->ob_dx;
			}
		}
		if ((ob->ob_orient && FIXED_IP(ob->ob_ndy) > -10)
		 || (!ob->ob_orient && FIXED_IP(ob->ob_ndy) > -gminspeed)) {
			ob->ob_ndy -= FIXED_UNIT;
		}
		ob->ob_life = EXPLLIFE;
	}

	movexy(ob, &x, &y);

	if (!in_range(0, x, currgame->gm_max_x - 1) || y <= (int) ground[x]) {
		if (orient) {
			stopsound(ob);
		}
		deallobj(ob);
		return (false);
	}
	++ob->ob_hitcount;

	ob->ob_symbol = &symbol_debris[ob->ob_orient].sym[0];

	return y < MAX_Y;
}

bool movesmok(OBJECTS * obp)
{
	OBJECTS *ob;
	obstate_t planestate;

	ob = obp;

	planestate = ob->ob_owner->ob_state;

	--ob->ob_life;

	if (ob->ob_life <= 0
	 || (planestate != FALLING && planestate != CRASHED
	  && !PlaneIsWounded(planestate))) {
		deallobj(ob);
		return false;
	}
	ob->ob_symbol = &symbol_pixel;

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
	ob->ob_symbol = &symbol_flock[ob->ob_orient].sym[0];
	return true;
}

bool moveballoon(OBJECTS *ob)
{
	const original_ob_t *orig = ob->ob_original_ob;
	int x, y, dx, dy, f;
	int step;

	if (ob->ob_life == -1) {
		// TODO: Explosion animation?
		deallobj(ob);
		return false;
	}

	// The spotter in the balloon occasionally fires their pistol. But they
	// don't shoot upwards (bullets never come from the top of the balloon)
	if (ob->ob_state == FLYING && gamenum > 0 && (countmove % 7) == 0) {
		OBJECTS *plane = FindEnemyPlane(ob);
		if (plane && plane->ob_y < ob->ob_y) {
			initshot(ob, plane);
			ob->ob_firing = plane;
		}
	}

	// If we have two balloons next to each other, we don't want them to
	// move perfectly synchronized. So we use the X coordinate as a kind
	// of random element.
	step = countmove + orig->x;

	// We adjust the momentum on each frame in both x and y dimensions,
	// which makes the balloon "float" around in a randomish way. However,
	// since we step through each entry in the sine table, all movements
	// cancel out and we never drift out of the same area of the map.
	dx = SIN(step / 7) * 128;
	dy = SIN(step / 3) * 128;
	ob->ob_dx =  dx >> 16;
	ob->ob_ldx = dx & 0xffff;
	ob->ob_ndy = dy;
	movexy(ob, &x, &y);

	// Which way are we drifting?
	f = orig->orient * 3
	  + (dx >= 20000 ? 2 :
	     dx <= -20000 ? 0 : 1);

	ob->ob_symbol = &symbol_balloon[f].sym[orig->transform];
	return true;
}

static bool checkwall(OBJECTS *obp, int direction)
{
	int check_x, cnt;

	check_x = obp->ob_x;
	for (cnt = 0; cnt < 20; ++cnt) {
		if (!in_range(0, check_x, currgame->gm_max_x - 1)) {
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
		ob->ob_ndy = -ob->ob_ndy;
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

	ob->ob_symbol = &symbol_bird[ob->ob_orient].sym[0];
	if (!in_range(0, x, currgame->gm_max_x - 1)
	 || !in_range((int) ground[x] + 1, y, MAX_Y - 1)) {
		ob->ob_y -= FIXED_IP(ob->ob_ndy);
		ob->ob_life = -2;
		return false;
	}
	return true;
}

bool moveox(OBJECTS * ob)
{
	int transform = ob->ob_original_ob->transform;
	ob->ob_symbol = &symbol_ox[ob->ob_state != STANDING].sym[transform];
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
	ob->ob_dx = ob->ob_ndy = ob->ob_ldx = ob->ob_speed = 0;

	ob->ob_hitcount = ((abs(orig_ob->x - ob->ob_x) < SAFERESET)
	                && (abs(ob->ob_orig_y - ob->ob_y) < SAFERESET))
	    ? (MAXCRCOUNT << 1) : MAXCRCOUNT;

	return true;
}

bool hitpln(OBJECTS * obp)
{
	OBJECTS *ob;

	ob = obp;
	ob->ob_ldx = 0;
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
