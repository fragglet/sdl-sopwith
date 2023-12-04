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
//        swinit   -      SW initialization
//

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "pcsound.h"
#include "timer.h"
#include "video.h"

#include "sw.h"
#include "swasynio.h"
#include "swconf.h"
#include "swdisp.h"
#include "swinit.h"
#include "swgames.h"
#include "swmain.h"
#include "swmove.h"
#include "swobject.h"
#include "swsound.h"
#include "swsplat.h"
#include "swsymbol.h"
#include "swtitle.h"

static const original_ob_t *orig_planes[MAX_PLYR * 2];
GRNDTYPE *ground;

static int have_savescore = 0;
static score_t savescore;		/* save players score on restart  */
int starting_level = 0;

static char helptxt[] =
"\n"
PACKAGE_STRING "\n"
"Copyright (C) 1984, 1985, 1987 BMB Compuscience\n"
"Copyright (C) 1984-2000 David L. Clark\n"
"Copyright (C) 2001-2023 Simon Howard, Jesse Smith\n"
"Licensed under the GNU GPL v2.\n"
"\n"
"Usage:  sopwith [options]\n"
"The options are:\n"
"        -n :  novice single player\n"
"        -s :  single player\n"
"        -c :  single player against computer\n"
"        -q :  begin game with sound off\n"
"        -g#:  start at level #\n"
"        -e :  turn off big explosions\n"
"        -m <filename>: load new mission from file\n"
"\n"
"Video:\n"
"        -f    :  fullscreen\n"
"\n"
#ifdef TCPIP
"Networking: \n"
"        -l    :  listen for connection\n"
" -j <host>    :  connect to a listening host\n"
" -p <port>    :  use alternative TCP port\n"
#endif
;

static void initobjs(void)
{
	topobj.ob_xnext = topobj.ob_next = &botobj;
	botobj.ob_xprev = botobj.ob_prev = &topobj;
	topobj.ob_x = -32767;
	botobj.ob_x = 32767;

	objbot = objtop = deltop = delbot = NULL;
	objfree = NULL;
}


void initgrnd(void)
{
	free(ground);
	ground = calloc(currgame->gm_max_x, sizeof(GRNDTYPE));
	memcpy(ground, currgame->gm_ground,
	       sizeof(GRNDTYPE) * currgame->gm_max_x);
}

static void initseed(void)
{
	srand(clock());
	explseed = rand() % 65536;
}

void initdisp(bool reset)
{
	swclearsplats();
	if (!reset) {
		swtitlf();
	}
}


//
// object creation
//


static void initplanescore (score_t *score)
{
	score->planekills = 0;
	score->medals = 0x0000;
	score->valour = 0;
	score->killscore = 0;
	score->landings = 0;
	score->medals_nr = 0;
	score->ribbons_nr = 0;
}

#define SERVICE_KILLSCORE 5
#define COMPETENCE_KILLSCORE 25
#define VALOUR_PRELIMIT 175
#define VALOUR_LIMIT 250

static void give_medal(OBJECTS *ob, int medal_id)
{
	ob->ob_score.medalslist[ob->ob_score.medals_nr++] = medal_id;
}

static void give_ribbon(OBJECTS *ob, int ribbon_id)
{
	ob->ob_score.ribbons[ob->ob_score.ribbons_nr++] = ribbon_id;
}

static void get_awards(OBJECTS *ob)
{
	score_t *lsc = &ob->ob_lastscore;
	score_t *sc = &ob->ob_score;

	if (!(lsc->medals & MEDAL_PURPLEHEART)) {
		// We only award purple heart if wounded in combat (=hit by
		// bullet), not from a bird strike or flying into an ox
		if (ob->ob_score.combatwound) {
			sc->medals |= MEDAL_PURPLEHEART;
			give_medal(ob, MEDAL_ID_PURPLEHEART);
		}
	}

	if (!(lsc->medals & MEDAL_ACE)) {
		if (sc->planekills >= 5) {
			sc->medals |= MEDAL_ACE;
			give_ribbon(ob, RIBBON_ID_ACE);
		}
	}

	if (!(lsc->medals & MEDAL_TOPACE)) {
		if (sc->planekills >= 25) {
			sc->medals |= MEDAL_TOPACE;
			give_ribbon(ob, RIBBON_ID_TOPACE);
		}
	}

	if (!(lsc->medals & MEDAL_SERVICE)) {
		if (sc->killscore >= SERVICE_KILLSCORE) {
			sc->landings++;
		}

		if (sc->landings >= 3) {
			sc->medals |= MEDAL_SERVICE;
			give_ribbon(ob, RIBBON_ID_SERVICE);
		}
	}

	if (sc->killscore >= COMPETENCE_KILLSCORE) {
		if (!(lsc->medals & MEDAL_COMPETENCE)) {
			sc->medals |= MEDAL_COMPETENCE;
			give_medal(ob, MEDAL_ID_COMPETENCE);
		} else if (!(lsc->medals & MEDAL_COMPETENCE2)) {
			sc->medals |= MEDAL_COMPETENCE2;
			give_ribbon(ob, RIBBON_ID_COMPETENCE2);
		}
	}

	if (!(lsc->medals & MEDAL_PREVALOUR)) {
		if (sc->valour >= VALOUR_PRELIMIT) {
			sc->medals |= MEDAL_PREVALOUR;
			give_ribbon(ob, RIBBON_ID_PREVALOUR);
		}
	}

	if (!(lsc->medals & MEDAL_VALOUR)) {
		if (sc->valour >= VALOUR_LIMIT) {
			sc->medals |= MEDAL_VALOUR;
			give_medal(ob, MEDAL_ID_VALOUR);
		}
	}

	sc->killscore = 0;
	ob->ob_lastscore = ob->ob_score;
}

static void get_endlevel(OBJECTS *ob)
{
	score_t *sc = &ob->ob_score;
	score_t *lsc = &ob->ob_lastscore;

	get_awards(ob);

	if (!(lsc->medals & MEDAL_PERFECT)) {
		if (ob->ob_crashcnt == 0) {
			sc->medals |= MEDAL_PERFECT;
			give_ribbon(ob, RIBBON_ID_PERFECT);
		}
	}
}

// plane

OBJECTS *initpln(OBJECTS * obp)
{
	OBJECTS *ob;
	int x, height, minx, maxx;

	if (!obp) {
		assert(num_planes < MAX_PLYR);
		ob = allocobj();
		ob->ob_original_ob = orig_planes[num_planes];
		planes[num_planes] = ob;
		++num_planes;
	} else {
		ob = obp;
	}

	if (obp && ob->ob_state != CRASHED && !ob->ob_athome) {
		/* Just returned home */
		get_awards(ob);
	}

	ob->ob_type = PLANE;

	ob->ob_x = ob->ob_original_ob->x;
	minx = ob->ob_x;
	maxx = ob->ob_x + 20;
	height = 0;
	for (x = minx; x <= maxx; ++x) {
		if (ground[x] > height) {
			height = ground[x];
		}
	}
	ob->ob_y = height + 13;
	ob->ob_orig_y = ob->ob_y;

	ob->ob_lx = ob->ob_ly = ob->ob_speed = ob->ob_flaps = ob->ob_accel
	    = ob->ob_hitcount = ob->ob_bdelay = ob->ob_mdelay
	    = ob->ob_bsdelay = 0;
	setdxdy(ob, 0, 0);
	ob->ob_orient = ob->ob_original_ob->orient;
	ob->ob_angle = (ob->ob_orient) ? (ANGLES / 2) : 0;
	ob->ob_target = ob->ob_missiletarget = NULL;
	ob->ob_firing = ob->ob_mfiring = NULL;
	ob->ob_bombing = ob->ob_bfiring = ob->ob_home = false;
	ob->ob_newsym = symbol_plane[ob->ob_orient][0];
	ob->ob_athome = true;
	ob->ob_onmap = true;
	ob->ob_score.combatwound = false;

	if (!obp || ob->ob_state == CRASHED) {
		/* New plane */
		ob->ob_rounds = MAXROUNDS;
		ob->ob_bombs = MAXBOMBS;
		ob->ob_missiles = MAXMISSILES;
		ob->ob_bursts = MAXBURSTS;
		ob->ob_life = MAXFUEL;
		initplanescore(&ob->ob_lastscore);
	}
	if (!obp) {
		initplanescore(&ob->ob_score);
		ob->ob_score.score = ob->ob_crashcnt = 0;
		ob->ob_endsts = PLAYING;
		ob->ob_target = NULL;
		insertx(ob, &topobj);
	} else {
		deletex(ob);
		insertx(ob, ob->ob_xnext);
	}

	ob->ob_state = FLYING;
	ob->ob_goingsun = false;

	return ob;
}



// player

void initplyr(OBJECTS * obp)
{
	OBJECTS *ob;

	ob = initpln(obp);
	if (!obp) {
		ob->ob_drawf = dispplyr;
		ob->ob_movef = moveplyr;
		ob->ob_clr = ob->ob_original_ob->owner;
		ob->ob_owner = ob;
		endcount = 0;

		ob->ob_plrnum = num_players;
		++num_players;

		/* todo: save pointers to all player planes
		 * and turn consoleplayer into a macro */

		if (ob->ob_plrnum == player) {
			consoleplayer = ob;
		}
	}
}

// computer opponent

void initcomp(OBJECTS * obp)
{
	OBJECTS *ob;

	ob = initpln(obp);
	if (!obp) {
		ob->ob_drawf = dispcomp;
		ob->ob_movef = movecomp;
		ob->ob_clr = 2;
		ob->ob_owner = planes[1];
	}
	if (playmode == PLAYMODE_SINGLE || playmode == PLAYMODE_NOVICE) {
		ob->ob_state = FINISHED;
		ob->ob_onmap = false;
		deletex(ob);
	}
}


static int isrange(int x, int y, int ax, int ay)
{
	int dx, dy, t;

	dy = abs(y - ay);
	dy += dy >> 1;
	dx = abs(x - ax);

	if (dx > 100 || dy > 100) {
		return -1;
	}

	if (dx < dy) {
		t = dx;
		dx = dy;
		dy = t;
	}

	return (7 * dx + 4 * dy) / 8;
}

// bullet

void initshot(OBJECTS *obo, OBJECTS * targ)
{
	OBJECTS *ob;
	int nangle, nspeed, dx, dy, r, bspeed, x, y;

	if (!targ && !compplane && !obo->ob_rounds) {
		return;
	}

	ob = allocobj();

	if (!ob) {
		return;
	}

	if (playmode != PLAYMODE_NOVICE) {
		--obo->ob_rounds;
	}

	bspeed = BULSPEED + gamenum;

	if (targ) {
		x = targ->ob_x + (targ->ob_dx << 2);
		y = targ->ob_y + (targ->ob_dy << 2);
		dx = x - obo->ob_x;
		dy = y - obo->ob_y;

		r = isrange(x, y, obo->ob_x, obo->ob_y);
		if (r < 1) {
			deallobj(ob);
			return;
		}
		ob->ob_dx = (dx * bspeed) / r;
		ob->ob_dy = (dy * bspeed) / r;
		ob->ob_ldx = ob->ob_ldy = 0;
	} else {
		nspeed = obo->ob_speed + bspeed;
		nangle = obo->ob_angle;
		setdxdy(ob, nspeed * COS(nangle), nspeed * SIN(nangle));
	}

	ob->ob_type = SHOT;
	ob->ob_x = obo->ob_x + SYM_WDTH / 2;
	ob->ob_y = obo->ob_y - SYM_HGHT / 2;
	ob->ob_lx = obo->ob_lx;
	ob->ob_ly = obo->ob_ly;

	ob->ob_life = BULLIFE;
	ob->ob_owner = obo;
	ob->ob_clr = obo->ob_clr;
	ob->ob_newsym = &symbol_pixel;
	ob->ob_drawf = NULL;
	ob->ob_movef = moveshot;
	ob->ob_speed = 0;

	insertx(ob, obo);

}

// bomb

void initbomb(OBJECTS *obo)
{
	OBJECTS *ob;
	int angle;

	if ((!compplane && !obo->ob_bombs) || obo->ob_bdelay) {
		return;
	}

	ob = allocobj();
	if (!ob) {
		return;
	}

	if (playmode != PLAYMODE_NOVICE) {
		--obo->ob_bombs;
	}

	obo->ob_bdelay = 10;

	ob->ob_type = BOMB;
	ob->ob_state = FALLING;
	ob->ob_dx = obo->ob_dx;
	ob->ob_dy = obo->ob_dy;
	ob->ob_onmap = true;

	if (obo->ob_orient) {
		angle = (obo->ob_angle + (ANGLES / 4)) % ANGLES;
	} else {
		angle = (obo->ob_angle + (3 * ANGLES / 4)) % ANGLES;
	}

	ob->ob_x = obo->ob_x + ((COS(angle) * 10) >> 8) + 4;
	ob->ob_y = obo->ob_y + ((SIN(angle) * 10) >> 8) - 4;
	ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;

	ob->ob_life = BOMBLIFE;
	ob->ob_owner = obo;
	ob->ob_clr = obo->ob_clr;
	ob->ob_newsym = symbol_bomb[0];
	ob->ob_drawf = dispbomb;
	ob->ob_movef = movebomb;

	insertx(ob, obo);

}

// missile

void initmiss(OBJECTS *obo)
{
	OBJECTS *ob;
	int angle, nspeed;

	if (obo->ob_mdelay || !obo->ob_missiles || !conf_missiles) {
		return;
	}

	ob = allocobj();
	if (!ob) {
		return;
	}

	if (playmode != PLAYMODE_NOVICE) {
		--obo->ob_missiles;
	}

	obo->ob_mdelay = 5;

	ob->ob_type = MISSILE;
	ob->ob_state = FLYING;

	angle = ob->ob_angle = obo->ob_angle;
	ob->ob_x = obo->ob_x + (COS(angle) >> 4) + 4;
	ob->ob_y = obo->ob_y + (SIN(angle) >> 4) - 4;
	ob->ob_lx = ob->ob_ly = 0;
	ob->ob_speed = nspeed = gmaxspeed + (gmaxspeed >> 1);
	setdxdy(ob, nspeed * COS(angle), nspeed * SIN(angle));

	ob->ob_life = MISSLIFE;
	ob->ob_owner = obo;
	ob->ob_clr = obo->ob_clr;
	ob->ob_newsym = symbol_missile[0];
	ob->ob_drawf = NULL;
	ob->ob_movef = movemiss;
	ob->ob_missiletarget = obo->ob_mfiring;
	ob->ob_orient = ob->ob_accel = ob->ob_flaps = 0;
	ob->ob_onmap = true;

	insertx(ob, obo);

}


// starburst

void initburst(OBJECTS *obo)
{
	OBJECTS *ob;
	int angle;

	if (obo->ob_bsdelay || !obo->ob_bursts || !conf_missiles) {
		return;
	}

	ob = allocobj();
	if (!ob) {
		return;
	}

	ob->ob_bsdelay = 5;

	if (playmode != PLAYMODE_NOVICE) {
		--obo->ob_bursts;
	}

	ob->ob_type = STARBURST;
	ob->ob_state = FALLING;

	if (obo->ob_orient) {
		angle = (obo->ob_angle + (3 * ANGLES / 8)) % ANGLES;
	} else {
		angle = (obo->ob_angle + (5 * ANGLES / 8)) % ANGLES;
	}

	setdxdy(ob, gminspeed * COS(angle), gminspeed * SIN(angle));
	ob->ob_dx += obo->ob_dx;
	ob->ob_dy += obo->ob_dy;

	ob->ob_x = obo->ob_x + ((COS(angle) * 10) >> 10) + 4;
	ob->ob_y = obo->ob_y + ((SIN(angle) * 10) >> 10) - 4;
	ob->ob_lx = ob->ob_ly = 0;

	ob->ob_life = BURSTLIFE;
	ob->ob_owner = obo;
	ob->ob_clr = obo->ob_clr;
	ob->ob_newsym = symbol_burst[0];
	ob->ob_drawf = NULL;
	ob->ob_movef = moveburst;

	insertx(ob, obo);

}

// building/target

static OBJECTS *inittarget(const original_ob_t *orig_ob)
{
	OBJECTS *ob;
	int x;
	int minh, maxh, aveh, minx, maxx;

	ob = allocobj();
	minx = ob->ob_x = orig_ob->x;
	maxx = ob->ob_x + 15;
	minh = 999;
	maxh = 0;
	for (x = minx; x <= maxx; ++x) {
		if (ground[x] > maxh) {
			maxh = ground[x];
		}
		if (ground[x] < minh) {
			minh = ground[x];
		}
	}
	aveh = (minh + maxh) / 2;

	while ((ob->ob_y = aveh + 16) >= MAX_Y) {
		--aveh;
	}

	for (x = minx; x <= maxx; ++x) {
		ground[x] = aveh;
	}

	ob->ob_dx = ob->ob_dy = ob->ob_lx = ob->ob_ly = ob->ob_ldx
	    = ob->ob_ldy = ob->ob_angle = ob->ob_hitcount = 0;
	ob->ob_type = TARGET;
	ob->ob_state = STANDING;
	ob->ob_orient = orig_ob->orient;

	// TODO: For the time being, player 3 is just interpreted as a
	// special player number that either maps to player 1 or 2
	// depending on the game mode; other players are just synonyms
	// for players 1 and 2.
	// This may change in the future if we support multiplayer with
	// more than two players.
	switch (orig_ob->owner) {
		case OWNER_NONE:
		case OWNER_PLAYER1:
		case OWNER_PLAYER5:
		case OWNER_PLAYER7:
			ob->ob_owner = planes[0];
			++numtarg[0];
			break;
		case OWNER_PLAYER2:
		case OWNER_PLAYER4:
		case OWNER_PLAYER6:
		case OWNER_PLAYER8:
			ob->ob_owner = planes[1];
			++numtarg[1];
			break;
		case OWNER_PLAYER3:
			if (playmode == PLAYMODE_ASYNCH) {
				ob->ob_owner = planes[0];
				++numtarg[0];
			} else {
				ob->ob_owner = planes[1];
				++numtarg[1];
			}
			break;
	}
	ob->ob_clr = ob->ob_owner->ob_clr;
	ob->ob_newsym = symbol_targets[0];
	ob->ob_drawf = disptarg;
	ob->ob_movef = movetarg;
	ob->ob_onmap = true;

	return ob;
}

// explosion

void initexpl(OBJECTS *obo, int small)
{
	OBJECTS *ob;
	int i, ic, speed;
	// int life;
	int obox, oboy, obodx, obody, oboclr;
	obtype_t obotype;
	bool mansym;
	int orient;

	obox = obo->ob_x + (obo->ob_newsym->w / 2);
	oboy = obo->ob_y + (obo->ob_newsym->h / 2);
	obodx = obo->ob_dx >> 2;
	obody = obo->ob_dy >> 2;
	oboclr = obo->ob_clr;

	obotype = obo->ob_type;
	if (obotype == TARGET && obo->ob_orient == 2) {
		ic = 1;
		// sdh: Oil tank explosions were changed in Sopwith "Network
		// Edition" to be much less intense; we make this change
		// optional. I believe +4 here is correct, because the amj
		// decompile says something like "speed = level + 8" and
		// gminspeed = level + MIN_SPEED = level + 4.
		if (conf_big_explosions) {
			speed = gminspeed + 4;
		} else {
			speed = gminspeed;
		}
	} else {
		ic = small ? 6 : 2;
		speed = gminspeed >> ((explseed & 7) != 7);
	}
	mansym = obotype == PLANE
		 && (obo->ob_state == FLYING || obo->ob_state == WOUNDED);

	for (i = 1; i <= 15; i += ic) {
		ob = allocobj();
		if (!ob) {
			return;
		}

		ob->ob_type = EXPLOSION;

		setdxdy(ob, COS(i) * speed, SIN(i) * speed);
		ob->ob_dx += obodx;
		ob->ob_dy += obody;

		ob->ob_x = obox + ob->ob_dx;
		ob->ob_y = oboy + ob->ob_dy;
		explseed *= ob->ob_x * ob->ob_y;
		explseed += 7491;
		if (!explseed) {
			explseed = 74917777;
		}

		ob->ob_life = EXPLLIFE;
		orient = ob->ob_orient = (explseed & 0x01C0) >> 6;
		if (mansym && (!orient || orient == 7)) {
			mansym = orient = ob->ob_orient = 0;
			ob->ob_dx = obodx;
			ob->ob_dy = -gminspeed;
		}

		ob->ob_lx = ob->ob_ly = ob->ob_hitcount = ob->ob_speed = 0;
		ob->ob_owner = obo;
		ob->ob_clr = oboclr;
		ob->ob_newsym = symbol_debris[0];
		ob->ob_drawf = dispexpl;
		ob->ob_movef = moveexpl;

		if (orient) {
			initsound(ob, S_EXPLOSION);
		}

		insertx(ob, obo);
	}
}

// smoke from falling plane

void initsmok(OBJECTS *obo)
{
	OBJECTS *ob;

	ob = allocobj();
	if (!ob) {
		return;
	}

	ob->ob_type = SMOKE;

	ob->ob_x = obo->ob_x + 8;
	ob->ob_y = obo->ob_y - 8;
	ob->ob_dx = obo->ob_dx;
	ob->ob_dy = obo->ob_dy;
	ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;
	ob->ob_life = SMOKELIFE;
	ob->ob_owner = obo;
	ob->ob_drawf = NULL;
	ob->ob_movef = movesmok;
	ob->ob_clr = obo->ob_clr;
}

// birds

static OBJECTS *initflock(const original_ob_t *orig_ob)
{
	OBJECTS *ob;
	int j;

	if (playmode == PLAYMODE_NOVICE || !conf_animals) {
		return NULL;
	}

	ob = allocobj();
	if (!ob) {
		return NULL;
	}

	ob->ob_type = FLOCK;
	ob->ob_state = FLYING;
	ob->ob_x = orig_ob->x;
	ob->ob_y = MAX_Y - 1;
	ob->ob_dx = ob->ob_x < (currgame->gm_max_x / 2) ? 2 : -2;
	ob->ob_dy = ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;
	ob->ob_orient = 0;
	ob->ob_life = FLOCKLIFE;
	ob->ob_owner = ob;
	ob->ob_newsym = symbol_flock[0];
	ob->ob_drawf = NULL;
	ob->ob_movef = moveflck;
	ob->ob_clr = 1;
	ob->ob_onmap = true;

	for (j = 0; j < NUM_STRAY_BIRDS; ++j) {
		initbird(ob, 1);
	}

	return ob;
}

// single bird

void initbird(OBJECTS *obo, int i)
{
	OBJECTS *ob;
	static int ibx[] = { 8, 3, 0, 6, 7, 14, 10, 12 };
	static int iby[] = { 16, 1, 8, 3, 12, 10, 7, 14 };
	static int ibdx[] = { -2, 2, -3, 3, -1, 1, 0, 0 };
	static int ibdy[] = { -1, -2, -1, -2, -1, -2, -1, -2 };

	ob = allocobj();
	if (!ob) {
		return;
	}

	ob->ob_type = BIRD;

	ob->ob_x = obo->ob_x + ibx[i];
	ob->ob_y = obo->ob_y - iby[i];
	ob->ob_dx = ibdx[i];
	ob->ob_dy = ibdy[i];
	ob->ob_orient = ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy =
	    0;
	ob->ob_life = BIRDLIFE;
	ob->ob_owner = obo;
	ob->ob_newsym = symbol_bird[0];
	ob->ob_drawf = NULL;
	ob->ob_movef = movebird;
	ob->ob_clr = obo->ob_clr;
	insertx(ob, &topobj);
}

// oxen

static OBJECTS *initox(const original_ob_t *orig_ob)
{
	OBJECTS *ob;

	if (playmode == PLAYMODE_NOVICE || !conf_animals) {
		return NULL;
	}

	ob = allocobj();
	if (!ob) {
		return NULL;
	}

	ob->ob_type = OX;
	ob->ob_state = STANDING;
	ob->ob_x = orig_ob->x;
	ob->ob_y = ground[ob->ob_x] + 16;
	ob->ob_orient = ob->ob_lx = ob->ob_ly = ob->ob_ldx =
	    ob->ob_ldy = ob->ob_dx = ob->ob_dy = 0;
	ob->ob_owner = ob;
	ob->ob_newsym = symbol_ox[0];
	ob->ob_drawf = NULL;
	ob->ob_movef = moveox;
	ob->ob_clr = 1;
	return ob;
}

static void inittargets(void)
{
	OBJECTS *ob;
	const original_ob_t *orig_ob;
	int i;

	numtarg[0] = 0;
	numtarg[1] = 0;

	for (i = 0; i < currgame->gm_num_objects; i++) {
		orig_ob = &currgame->gm_objects[i];
		switch (orig_ob->type) {
			case TARGET:
				ob = inittarget(orig_ob);
				break;
			case OX:
				ob = initox(orig_ob);
				break;
			case FLOCK:
				ob = initflock(orig_ob);
				break;
			default:
				continue;
		}

		if (ob != NULL) {
			ob->ob_original_ob = orig_ob;
			insertx(ob, &topobj);
		}
	}
}

static void initgdep(void)
{
	gmaxspeed = MAX_SPEED + gamenum;
	gminspeed = MIN_SPEED + gamenum;

	targrnge = 150;
	if (gamenum < 6) {
		targrnge -= 15 * (6 - gamenum);
	}
	targrnge *= targrnge;
}

void swinitlevel(void)
{
	int i;

	if (have_custom_level) {
		currgame = &custom_level;
	} else {
		currgame = &original_level;
	}

	// clear out any waiting keys. this stops, eg. 's' on the
	// menu from toggling sound once the game starts

	Vid_GetGameKeys();

	if (playmode == PLAYMODE_ASYNCH) {
		init1asy();
	}

	swclearsplats();

	initsndt();
	initgrnd();
	initobjs();

	num_players = 0;
	num_planes = 0;
	memset(planes, 0, sizeof(planes));
	memset(orig_planes, 0, sizeof(orig_planes));

	for (i = 0; i < currgame->gm_num_objects; i++) {
		int pln_index;
		if (currgame->gm_objects[i].type != PLANE) {
			continue;
		}
		pln_index = currgame->gm_objects[i].owner - 1;
		if (pln_index >= 0 && pln_index < MAX_PLYR * 2) {
			orig_planes[pln_index] = &currgame->gm_objects[i];
		}
	}

	if (keydelay == -1) {
		keydelay = 1;
	}

	if (playmode == PLAYMODE_ASYNCH) {
		maxcrash = MAXCRASH * 2;
		init2asy();
	} else {
		maxcrash = MAXCRASH;

		// single player. we spawn as many enemy planes as we have
		// defined within the map.
		// TODO: We should have a way to define the number of enemy
		// planes in levels.
		initplyr(NULL);
		for (i = 1; i < MAX_PLYR && orig_planes[i] != NULL; i++) {
			initcomp(NULL);
		}
	}

	inittargets();

	initdisp(false);
	initgdep();

	// sdh 16/11/2001: this needs to be reset with each new game
	// to keep netgames in sync if we have already played

	countmove = 0;
	successful_flight = false;

	for (i=0; i<num_players; ++i) {
		latest_player_time[i] = 0;
	}
}

void swrestart(void)
{
	OBJECTS *ob;
	int inc;
	int time;
		
	if (consoleplayer->ob_endsts == WINNER) {
		ob = planes[player];
		inc = 0;

		get_endlevel(ob);

		while (ob->ob_crashcnt < maxcrash) {
			++ob->ob_crashcnt;
			inc += 25;
			ob->ob_score.score += inc;

			Vid_Update();
			
			time = Timer_GetMS();
			while (Timer_GetMS() < time + 200);
		}
		++gamenum;
		savescore = ob->ob_score;
		have_savescore = 1;
	} else {
		// gamenum = 0;
		// allow variable start level -- Jesse
		gamenum = starting_level;
		have_savescore = 0;

		// sh 28/10/2001: go back to the title screen

		playmode = PLAYMODE_UNSET;
	}

	longjmp(envrestart, 0);
}

// init game

void swinit(int argc, char *argv[])
{
	bool n = false;
	bool s = false;
	bool c = false;
	bool a = false;
	bool k = false;
	int modeset = 0, keyset;
	int i;

	soundflg = 0;
	// DEBIAN FOLKS: please don't disable the configuration file.
	// The code has changed significantly, that patch should no
	// longer be necessary.
	swloadconf();

	for (i=1; i<argc; ++i) {
		if (!strcasecmp(argv[i], "-n")) {
			n = 1;
		} else if (!strcasecmp(argv[i], "-s")) {
			s = 1;
		} else if (!strcasecmp(argv[i], "-c")) {
			c = 1;
		} else if (!strcasecmp(argv[i], "-f")) {
			vid_fullscreen = 1;
		} else if (!strncasecmp(argv[i], "-g", 2)) {
			sscanf(& (argv[i][2]), "%d", &starting_level);
			gamenum = starting_level;
		} else if (!strcasecmp(argv[i], "-q")) {
			soundflg = 1;
		} else if (!strcmp(argv[i], "-m") && (i + 1) < argc) {
			load_custom_level(argv[i + 1]);
			++i;
		} else
#ifdef TCPIP
		if (!strcasecmp(argv[i], "-l")) {
			a = 1;
			asynmode = ASYN_LISTEN;
		} else if (!strcasecmp(argv[i], "-j")) {
			if (i + 1 >= argc) {
				error_exit("insufficient arguments to -j");
			}
			a = 1;
			asynmode = ASYN_CONNECT;
			++i;
			snprintf(asynhost, sizeof(asynhost), "%s", argv[i]);
		} else if (!strcmp(argv[i], "-p")) {
			if (i + 1 >= argc) {
				error_exit("insufficient arguments to -p");
			}
			++i;
			asynport = atoi(argv[i]);
		} else
#endif
		{
			puts(helptxt);
			exit(0);
		}
	}

	modeset = n | s | c | a;
	keyset = k;

	soundflg = !soundflg;
	if (modeset && keyset) {
		titleflg = true;
	}

	initseed();

	// sdh 27/6/2002: generate symbol objects

	symbol_generate();

	Timer_Init();
	Vid_Init();
	Vid_SetVideoPalette(conf_video_palette);

	// dont init speaker if started with -q (quiet)
	if (soundflg) {
		Speaker_Init();		// init pc speaker
	}

	initsndt();
	
	// set playmode if we can, from command line options

	playmode =
		n ? PLAYMODE_NOVICE :
		s ? PLAYMODE_SINGLE :
		c ? PLAYMODE_COMPUTER :
		a ? PLAYMODE_ASYNCH :
		PLAYMODE_UNSET;
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/06/2002: move symbols to new sopsym_t format,
//                 remove references to symwdt, symhgt
// sdh 27/03/2002: move old drawing functions to new ones (pntsym, pntcol)
// sdh 26/03/2002: change CGA_ to Vid_
// sdh 16/11/2001: TCPIP #define to disable TCP/IP support
// sdh 16/11/2001: fix out of sync error
// sdh 29/10/2001: load game options from config file
// sdh 28/10/2001: extra game options
// sdh 28/10/2001: game init restructured: swinitlevel now initialises the
//                 level, getmode is called from main
// sdh 26/10/2001: merge guages into a single function
// sdh 24/10/2001: fix auxdisp buffer
// sdh 23/10/2001: fixed arguments help list and rewrote argument checking
//                 to support new network features
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to be more readable
// sdh 19/10/2001: removed all extern definitions, these are now in headers
//                 shuffled some functions around to shut up compiler
// sdh 18/10/2001: converted all functions in this file to ANSI-style
// 		   arguments from k&r
//
// 2000-10-29      Copyright update.
//                 Comment out multiplayer selection on startup.
// 99-01-24        1999 copyright.
//                 Disable network support.
// 96-12-26        New network version.
//                 Remove keyboard prompts. Speed up game a bit.
// 87-04-09        Fix to initial missile path.
//                 Delay between starbursts
// 87-04-06        Computer plane avoiding oxen.
// 87-04-05        Less x-movement in explosions
// 87-04-04        Missile and starburst support
// 87-03-31        Less x-movement in explosions
// 87-03-31        Missiles
// 87-03-30        Novice player.
// 87-03-12        Wounded airplanes.
// 87-03-11        Smaller fuel tank explosions.
// 87-03-09        Microsoft compiler.
// 87-01-09        BMB standard help text.
//                 Multiple serial ports.
// 85-10-31        Atari
// 84-02-02        Development
//
