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
//        sw.h     -      SW  Manifest Definitions
//

#ifndef __SW_H__
#define __SW_H__

#include "std.h"
#include <setjmp.h>
#include "swsymbol.h"
#include "config.h"

// We only enable TCP/IP support if we have the appropriate socket header
// files. Emscripten doesn't get TCP/IP.
#if !defined(__EMSCRIPTEN__) && ( \
    defined(HAVE_NETINET_IN_H) || defined(HAVE_WINSOCK_H))
#define TCPIP
#endif

// In Emscripten mode there's nothing we can exit to. This might also be
// useful when porting to certain systems (eg. games consoles).
#ifdef __EMSCRIPTEN__
#define NO_EXIT
#endif

// In Emcripten mode we don't bother with fullscreen mode. This is also
// something likely useful for games consoles.
#ifdef __EMSCRIPTEN__
#define NO_FULLSCREEN
#endif

#ifdef _MSC_VER
#define inline __inline
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/*  Constants  */

#define FPS             10              /* How fast the game runs */

#define DEFAULT_PORT    3847            /* Default TCP port for multiplayer */

#define MAX_Y           200             /*  Maximum Y coordinate            */
#define MIN_SPEED       4               /*  Minimum plane speed             */
#define MAX_SPEED       8               /*  Maximum plane speed             */
#define MAX_THROTTLE    4               /*  Maximum acceleration            */

#define MAXCRCOUNT      10              /*  Number of turns as crashed      */
#define FALLCOUNT       10              /*  Moves between falling plane adj */
#define STALLCOUNT      6               /*  Moves between stalling plane adj*/
#define TARGHITCOUNT    10              /*  Target hit count before explod'n*/

#define SCR_WDTH        320             /*  Screen Width                    */
#define SCR_HGHT        200             /*  Screen Height                   */

#define SCR_CENTR       ((SCR_WDTH / 2) - 8) /*  Centre column of screen */
#define SCR_MNSH        16              /*  Minumum line number to shift    */
#define SCR_MXSH        75              /*  Maximum line number to shift    */

// sdh 27/7/2002: make these independent of the screen width

#define WRLD_RSX        (currgame->gm_max_x / 320 * 2 + 1) /*  World display X and  */
#define WRLD_RSY        (MAX_Y / SCR_MNSH + 1)     /*  Y divisors           */

#define GAUGEX          (SCR_CENTR - 25)  /* X co-ordinate of first gauge */

typedef enum {
	PLAYMODE_UNSET = 0,
	PLAYMODE_SINGLE,
	PLAYMODE_COMPUTER,
	PLAYMODE_ASYNCH,
	PLAYMODE_NOVICE
} playmode_t;

#define MAXROUNDS       200             /* Maximum number of rounds of shot */
#define MAXBOMBS        5               /* Maximum number of bombs available*/
#define MAXMISSILES     5               /* Maximum number of missiles       */
#define MAXBURSTS       5               /* Maximum nuber of starbursts      */
#define MAXFUEL         9000            /* Maximum number of turns between  */
                                        /*  refuellings                     */
#define MAXCRASH        5               /* Mximum number of crashes allowed */

#define MAX_PLYR        4               /* Maximum number of players        */
#define MAX_OBJS        100             /* Maximum number of objects        */
#define NUM_STRAY_BIRDS 1               /* Number of stray birds per flock  */

#define ANGLES          16              /*  Number of angle increments      */
#define ORIENTS         2               /*  Number of plane orientations    */
#define SYM_WDTH        16              /*  Symbol width in pixels          */
#define SYM_HGHT        16              /*  Symbol height in pixels         */

#define BULSPEED        10              /* Bullet speed    */
#define BULLIFE         10              /* Bullet life     */
#define BOMBLIFE        5               /* Moves between bomb course adj   */
#define MISSLIFE        50              /* Missile life                    */
#define BURSTLIFE       20              /* Starburst life                  */
#define EXPLLIFE        3               /* Moves between explosion "  "    */
#define SMOKELIFE       10              /* Smoke life                      */
#define BIRDLIFE        4               /* Moves between bird flaps        */
#define FLOCKLIFE       5               /* Moves between flop flaps        */

typedef enum {               /*  Player states  */
	WAITING = 0,
	FLYING ,
	HIT,
	CRASHED,
	FALLING,
	STANDING,
	STALLED,
	REBUILDING,
	WOUNDED,
	WOUNDSTALL,

	FINISHED = 91,
} obstate_t;

typedef enum {
	PLAYING,
	WINNER,
	LOSER
} obendstatus_t;

typedef enum {
	GROUND = 0,
	PLANE,
	BOMB,
	SHOT,
	TARGET,
	EXPLOSION,
	SMOKE,
	FLOCK,
	BIRD,
	OX,
	MISSILE,
	STARBURST,
	DUMMYTYPE = 99,
} obtype_t;

#define NEAR            ( 150 * 150 )   /* Computer control distances        */
#define CLOSE           32
#define HOME            16
#define SAFERESET       32

#define QUIT            -5000           /* Plane life value when quitting    */

#define K_ACCEL         0x0001          /* Keyboard word masks               */
#define K_DEACC         0x0002
#define K_FLAPU         0x0004
#define K_FLAPD         0x0008
#define K_FLIP          0x0010
#define K_SHOT          0x0020
#define K_HARRYKEYS     0x0040          /* reverse flapu/d when flipped */
#define K_BOMB          0x0100
#define K_HOME          0x0200
#define K_SOUND         0x0400
#define K_BREAK         0x0800
#define K_MISSILE       0x1000
#define K_STARBURST     0x2000

typedef  unsigned int GRNDTYPE;

struct tt {                     /*  Continuous tone table entry    */
	unsigned  tt_tone;
	unsigned  tt_chng;
	struct tt *tt_next;
	struct tt *tt_prev;
};

typedef struct tt TONETAB;

#define MEDAL_ID_PURPLEHEART 0
#define MEDAL_ID_COMPETENCE 1
#define MEDAL_ID_VALOUR 2

#define RIBBON_ID_ACE 0
#define RIBBON_ID_TOPACE 1
#define RIBBON_ID_PERFECT 2
#define RIBBON_ID_SERVICE 3
#define RIBBON_ID_COMPETENCE2 4
#define RIBBON_ID_PREVALOUR 5

typedef struct {
	int		score;
	int		planekills;	/* # of planes shot down with this plane */
	int		valour;		/* Points for valourous conduct */
	int		killscore;	/* Competence count */
	int		landings;	/* Landings with nontrivial competence */
	int		medals_nr;	/* Awarded # of medals */
	int		medalslist[3];	/* Medal display order */
	int		ribbons_nr;	/* Awarded # of ribbons */
	int		ribbons[6];	/* Ribbon display order */
	unsigned int	medals;		/* See below */
	bool            combatwound;    /* If true, wounding came from a bullet */
} score_t;

#define MEDAL_PURPLEHEART	(1<<0)	/* Returned when damaged */
#define MEDAL_ACE		(1<<1)	/* Five plane kills, returned to base */
#define MEDAL_TOPACE		(1<<2)	/* 25 plane kills, returned to base */
#define MEDAL_PERFECT		(1<<3)  /* Finished one stage with full planes */
#define MEDAL_SERVICE		(1<<4)  /* Returned three times after doing some damage */
#define MEDAL_COMPETENCE	(1<<5)	/* Dealt nontrivial amount of damage to the enemy */
#define MEDAL_COMPETENCE2	(1<<6)	/* 2nd Competence medal (ribbon) */
#define MEDAL_PREVALOUR		(1<<7)	/* A ribbon awarded about half-way towards MEDAL_VALOUR */
#define MEDAL_VALOUR		(1<<8)	/* The Iron Cross / Victoria Cross for getting enough valour points */

// Objects in levels are assigned an "owner" player. This is mainly used
// to control which targets are assigned to which player. However, since
// each player is assigned a unique color scheme, these constants are
// also used in some places to denote a color scheme to draw symbols in.
// For the time being, OWNER_PLAYER3 means "OWNER_PLAYER1 in multiplayer;
// OWNER_PLAYER2 in single player". All others are synonyms for either
// OWNER_PLAYER1 or OWNER_PLAYER2; this may change in the future if/when
// we support more than two players.
typedef enum {
	OWNER_NONE,
	OWNER_PLAYER1,
	OWNER_PLAYER2,
	OWNER_PLAYER3,
	OWNER_PLAYER4,
	OWNER_PLAYER5,
	OWNER_PLAYER6,
	OWNER_PLAYER7,
	OWNER_PLAYER8,
} ob_owner_t;

typedef struct {
	obtype_t type;
	int x;
	int orient;
	int territory_l, territory_r;    /* Computer plane territory */
	ob_owner_t owner;
} original_ob_t;

typedef struct obj {                            /*  Object list             */
	obstate_t      ob_state;
	int            ob_x, ob_y;
	int            ob_dx, ob_dy;
	int            ob_angle;
	int            ob_orient;
	int            ob_speed;
	int            ob_accel;
	int            ob_flaps;
	struct obj    *ob_firing;
	int            ob_rounds;
	int            ob_hitcount;
	int            ob_life;
	struct obj    *ob_owner;
	struct obj    *ob_target;
	int            ob_bombing;
	int            ob_bombs;
	int            ob_clr;
	int            ob_lx, ob_ly;
	int            ob_ldx, ob_ldy;
	struct obj    *ob_next;
	struct obj    *ob_prev;
	bool           ob_drwflg;
	bool           ob_onmap;
	void        ( *ob_drawf ) (struct obj *);
	bool        ( *ob_movef ) (struct obj *);
	struct obj    *ob_xnext;
	struct obj    *ob_xprev;
	int            ob_crashcnt;
	sopsym_t      *ob_newsym;
	int            ob_bdelay;
	int            ob_home;
	obtype_t       ob_type;
	int            ob_athome;
	struct tt     *ob_sound;
	int            ob_missiles;
	struct obj    *ob_mfiring;
	int            ob_mdelay;
	struct obj    *ob_missiletarget;
	int            ob_bursts;
	int            ob_bfiring;
	int            ob_bsdelay;
	int            ob_plrnum;
	obendstatus_t  ob_endsts;
	bool           ob_goingsun;
	score_t	       ob_score;
	score_t	       ob_lastscore;
	const original_ob_t *ob_original_ob;
	// There is a non-orthogonality here. The original X position is
	// defined in ob_original_ob->x (ie. the definition of the level)
	// but the original Y position is not part of the level; it gets
	// generated automatically when the object first gets instantiated.
	// That generated values is saved here as ob_orig_y, and we must
	// look in two different locations to get the two coordinates.
	int            ob_orig_y;
} OBJECTS;

typedef struct {                                /*  Game structure          */
	unsigned int gm_rseed;
	original_ob_t *gm_objects;
	unsigned int gm_num_objects;
	GRNDTYPE *gm_ground;
	unsigned int gm_max_x;  // Defines length of gm_ground[] array.
} GAMES;

extern const int sintab[ANGLES];

static inline int COS(int x) {
	return sintab[(x+(ANGLES/4)) % ANGLES];
}

static inline int SIN(int x) {
	return sintab[x % ANGLES];
}

// Evalutes to length of static arrays.
#define arrlen(x) (sizeof(x) / sizeof(*x))

#endif

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/07/2002: made some constants independent of screen width
// sdh 27/06/2002: remove ob_symwdt and ob_symhgt. use new sopsym_t for
//                 sprites
// sdh 20/04/2002: change SCR_CENTR to be dependent on SCR_WDTH
//                 widescreen on revo(psion)
// sdh 16/11/2001: TCPIP #define to disable TCP/IP support
// sdh 21/10/2001: moved sound priorities into swsound.h
// sdh 21/10/2001: moved plane sprite constants into appropriate headers
// sdh 21/10/2001: created obstate_t and obtype_t enum types
// sdh 21/10/2001: rearranged headers, added cvs tags
//
// 87-04-08        Delay between starbursts
// 87-04-04        Missile and starburst support
// 87-03-31        Missiles
// 87-03-30        Novice Player
// 87-03-13        Splatted bird symbol
// 87-03-12        Crashed planes stay longer at home.
// 87-03-12        Wounded airplanes
// 87-03-09        Microsoft compiler
// 85-10-31        Atari
// 85-04-02        Asynch Mode
// 84-06-12        PCjr Speed-up
// 84-02-02        Development
//
