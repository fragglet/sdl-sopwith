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
//        sw.h     -      SW  Manifest Definitions
//
//---------------------------------------------------------------------------

// sdh: #ifndef to catch multiple includes

#ifndef __SW_H__
#define __SW_H__

#include "std.h"
#include <setjmp.h>
#include "swmach.h"
#include "swsymbol.h"
#include "config.h"

/*  Constants  */

#define MAX_X           3000            /*  Maximum X coordinate            */
#define MAX_Y           200             /*  Maximum Y coordinate            */
#define MIN_SPEED       4               /*  Minimum plane speed             */
#define MAX_SPEED       8               /*  Maximum plane speed             */
#define MAX_THROTTLE    4               /*  Maximum acceleration            */

#define MAXCRCOUNT      10              /*  Number of turns as crashed      */
#define FALLCOUNT       10              /*  Moves between falling plane adj */
#define STALLCOUNT      6               /*  Moves between stalling plane adj*/
#define TARGHITCOUNT    10              /*  Target hit count before explod'n*/

#ifdef PSION

// fill the screen on the revo

#define SCR_WDTH        480             /*  Screen Width                    */

#else

#define SCR_WDTH        320             /*  Screen Width                    */

#endif

#define SCR_HGHT        200             /*  Screen Height                   */

#define SCR_CENTR       ((SCR_WDTH / 2) - 8) /*  Centre column of screen */
#define SCR_SEGM        0xB800          /*  Screen Video segment            */
#define SCR_ROFF        0x2000          /*  Screen odd rastor line offset   */
#define SCR_LINW        80              /*  Screen line width in bytes      */
#define SCR_MNSH        16              /*  Minumum line number to shift    */
#define SCR_MXSH        75              /*  Maximum line number to shift    */
#define SCR_LIMIT       180             /*  Screen shift limits             */

// sdh 27/7/2002: make these independent of the screen width

#define WRLD_RSX        (MAX_X / 320 * 2 + 1) /*  World display X and  */
#define WRLD_RSY        (MAX_Y / SCR_MNSH + 1)     /*  Y divisors           */

#define MINFLCKX        370 			  /*  Bird flock travel    */
#define MAXFLCKX        (MAX_X - 370)             /*  limits */

// sdh 26/10/2001: merged guages, removed unneccesary coordinates

#define GUAGEX          (SCR_CENTR - 25)  /* X co-ordinate of first guage */
#define GHOSTX          (SCR_CENTR - 21)/*  Ghost face display x-coodinate  */

#ifdef  IBMPC
#define         PRINTINT        0x05    /* Shift print screen interrupt     */
#define         BREAKINT        0x1B    /* Control break interrupt          */
#define         CLOCKINT        0x1C    /* Clock interrupt                  */
#define         KEYINT          0x09    /* Keyboard interrupt               */
#endif
#ifdef  ATARI
#define         PRINTINT        0x05    /* Shift print screen interrupt     */
#define         BREAKINT        0x05    /* Control break interrupt          */
#define         CLOCKINT        0x100   /* Clock interrupt                  */
#define         KEYINT          0x46    /* Keyboard interrupt               */
#endif

#define PCDOS           0               /*  Operating environments          */
#define JRROMDOS        1
#define JRROM           2

// sdh 28/10/2001: made into a type

typedef enum {
	PLAYMODE_UNSET = 0,
	PLAYMODE_SINGLE,
	PLAYMODE_MULTIPLE,
	PLAYMODE_COMPUTER,
	PLAYMODE_ASYNCH,
	PLAYMODE_NOVICE
} playmode_t;

#define MAXROUNDS       200             /* Maximum number of rounds of shot */
#define MAXBOMBS        5               /* Maximum number of bombs available*/
#define MAXMISSILES     5               /* Maximum number of missiles       */
#define MAXBURSTS       5               /* Maximum nuber of starbursts      */
#define MAXFUEL         ( 3 * MAX_X )   /* Maximum Number of turns between  */
                                        /*  refuellings                     */
#define MAXCRASH        5               /* Mximum number of crashes allowed */

#define MAX_PLYR        4               /* Maximum number of players        */
#define MAX_TARG        20              /* Maximum number of targets        */
#define MAX_OBJS        100             /* Maximum number of objects        */
#define MAX_FLCK        4               /* Maximum number of flocks         */
#define MAX_BIRD        1               /* Maximum number of stray birds    */
                                        /*    per flock                     */
#define MAX_GAME        7               /* Maximum number of games          */
#define MAX_OXEN        2               /* Maximum number of oxen           */

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
	GHOST,
	GHOSTCRASHED,
	GHOSTSTALLED,
} obstate_t;

// sdh 21/10/2001: made this an enum

enum {
	WINNER = 1,
	LOSER
};

// sdh 21/10/2001: converted object types to an enum

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
#define NEARAPPROACH    200
#define CLOSE           32
#define DEADON          ( 4 * 4 )
#define HOME            16
#define SAFERESET       32

#define QUIT            -5000           /* Plane life value when quitting    */

#define K_ACCEL         0x0001          /* Keyboard word masks               */
#define K_DEACC         0x0002
#define K_FLAPU         0x0004
#define K_FLAPD         0x0008
#define K_FLIP          0x0010
#define K_SHOT          0x0020
#define K_BOMB          0x0100
#define K_HOME          0x0200
#define K_SOUND         0x0400
#define K_BREAK         0x0800
#define K_MISSILE       0x1000
#define K_STARBURST     0x2000

#define K_ASYNACK       0x40


#define COMM_FILE       "c:sopwith?.dta" /*  Multi user communications file  */
#define COMM_CMD        "c:semaphor\0   "/*  Multi-user semaphor file        */


// made these into typedefs rather than ugly #define macros

typedef int BIOFD;

                                        /*  Internal representation of ground */
                                        /*  due to non-implementation of      */
                                        /*  unsigned char on ATARI            */
typedef  unsigned int GRNDTYPE;

struct tt {                     /*  Continuous tone table entry    */
        unsigned  tt_tone;
        unsigned  tt_chng;
        struct tt *tt_next;
        struct tt *tt_prev;
};

typedef struct tt TONETAB;

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
        int            ob_score;
        int            ob_rounds;
        int            ob_hitcount;
        int            ob_updcount;
        int            ob_life;
        struct obj    *ob_owner;
        //int            ob_symhgt;   // sdh 27/6/2002
        //int            ob_symwdt;   // sdh 27/6/2002
        int            ob_bombing;
        int            ob_bombs;
        int            ob_clr;
        int            ob_lx, ob_ly;
        int            ob_ldx, ob_ldy;
        struct obj    *ob_next;
        struct obj    *ob_prev;
        int            ob_index;
        int            ob_oldx;
        int            ob_oldy;
        int            ob_drwflg;
        int            ob_delflg;
        // char          *ob_oldsym;   // sdh 27/6/2002
	sopsym_t      *ob_oldsym;
        void        ( *ob_drawf ) ();
        BOOL        ( *ob_movef ) ();
        struct obj    *ob_xnext;
        struct obj    *ob_xprev;
        int            ob_crashcnt;
        // char          *ob_newsym;   // sdh 27/6/2002
	sopsym_t      *ob_newsym;
        int            ob_bdelay;
        int            ob_home;
        int            ob_hx[3], ob_hy[3];
        obtype_t       ob_type;
        struct obj    *ob_dnext;
        int            ob_athome;
        struct tt     *ob_sound;
        int            ob_missiles;
        struct obj    *ob_mfiring;
        int            ob_mdelay;
        struct obj    *ob_target;
        int            ob_bursts;
        int            ob_bfiring;
        int            ob_bsdelay;
}       OBJECTS;

typedef struct {                                /*  Game structure          */
        int      gm_x[MAX_PLYR*2];
        int      gm_orient[MAX_PLYR*2];
        unsigned ( *gm_randf ) ();
        unsigned gm_rseed;
        int      ( *gm_specf ) ();
        int      gm_xtarg[MAX_TARG];
        int      gm_ttarg[MAX_TARG];
}       GAMES;

typedef struct {                                /*  Communications buffer   */
        unsigned mu_maxplyr;
        unsigned mu_numplyr;
        unsigned mu_lstplyr;
        unsigned mu_key[MAX_PLYR];
        unsigned mu_state[MAX_PLYR];
        unsigned mu_explseed;
}       MULTIO;

typedef struct {                                /*  Output message record   */
        char     msgo_cmd;
        char     msgo_port;
        char     msgo_ssgnl;
        char     msgo_rsgnl;
        char     msgo_1fill[6];
        MULTIO   msgo_buff;
        char     msgo_2fill[1];
}       MSGOUT;

typedef struct {                                /*  Input message record    */
        char     msgi_port;
        char     msgi_1fill[2];
        char     msgi_myport;
        char     msgi_2fill[6];
        MULTIO   msgi_buff;
        char     msgi_3fill[1];
}       MSGIN;



typedef struct {                        /*  Old display parameters for    */
        int     ow_xorplot;             /*  each object                   */
        int     ow_x, ow_y;
}       OLDWDISP;

// sdh: made these into inline functions rather than ugly #define macros

extern int sintab[];

static inline int COS(int x) {
	return sintab[(x+(ANGLES/4)) % ANGLES];
}

static inline int SIN(int x) {
	return sintab[x % ANGLES];
}

#endif

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:29  fraggle
// Initial revision
//
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
//---------------------------------------------------------------------------

