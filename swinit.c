/*
        swinit   -      SW initialization

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-02        Development
                        85-10-31        Atari
                        87-01-09        BMB standard help text.
                                        Multiple serial ports.
                        87-03-09        Microsoft compiler.
                        87-03-11        Smaller fuel tank explosions.
                        87-03-12        Wounded airplanes.
                        87-03-30        Novice player.
                        87-03-31        Missiles
                        87-03-31        Less x-movement in explosions
                        87-04-04        Missile and starburst support
                        87-04-05        Less x-movement in explosions
                        87-04-06        Computer plane avoiding oxen.
                        87-04-09        Fix to initial missile path.
                                        Delay between starbursts
                        96-12-26        New network version.
                                        Remove keyboard prompts.
                                        Speed up game a bit.
                        99-01-24        1999 copyright.
                                        Disable network support.
                        2000-10-29      Copyright update.
                                        Comment out multiplayer selection
                                          on startup.
*/
#include "sw.h"
#include "cgavideo.h"
#include "pcsound.h"

// sdh -- put them in a header damnit! aarrgh

extern  int     _systype;               /* operating environment            */
extern  int     playmode;               /* Mode of play                     */
extern  GAMES   swgames[], *currgame;   /* Game parameters and current game */
extern  GRNDTYPE ground[];              /* Ground height by pixel           */
extern  GRNDTYPE orground[];            /* Original ground height by pixel  */


extern  MULTIO  *multbuff;              /* Communications buffer            */

extern  BOOL    hires;                  /* High res flag                    */
extern  BOOL    titleflg;               /* Title flag                       */
extern  BOOL    disppos;                /* Display position flag            */
extern  int     keydelay;               /*  Number of displays per keystroke*/
extern  int     soundflg;               /*  Sound flag                      */
extern  BOOL    repflag;                /*  Report statistics flag          */
extern  BOOL    joystick;               /*  Joy stick being used            */
extern  BOOL    ibmkeybd;               /*  IBM keyboard or look-alike      */
extern  int     multtick;               /*  Multiple user tick delay        */
extern  BOOL    inplay;                 /*  Currently playing flag          */
extern  int     koveride;               /* Keyboard override index number   */
extern  int     missok;                 /* Missiles supported               */

extern  int     gamenum;                /* Current game number              */
extern  int     gmaxspeed, gminspeed;   /* Speed range based on game number */
extern  int     targrnge;               /* Target range based on game number*/

extern  int     displx, disprx;         /* Display left and right           */
extern  char    auxdisp[];              /* Auxiliary display area           */
extern  BOOL    dispinit;               /* Initalized display flag.         */
extern  char    *histin, *histout;      /* History input and output files   */

extern  OBJECTS *nobjects;              /* Objects list.                    */
extern  OBJECTS oobjects[];             /* Original plane object description*/
extern  OBJECTS *objbot, *objtop,       /* Top and bottom of object list    */
                *objfree,               /* Free list                        */
                *deltop, *delbot;       /* Newly deallocated objects        */
extern  OBJECTS *targets[];             /* Target status array              */
extern  int     numtarg[];              /* number of active targets         */
extern  OBJECTS topobj, botobj;         /* Top and Bottom of object by x lst*/
extern  int     player;                 /* Pointer to player's object       */
extern  BOOL    plyrplane;              /* Current object is the player     */
extern  BOOL    compplane;              /* Current object is a comp plane   */
extern  int     currobx;                /* Current object index             */
extern  OLDWDISP wdisp[];               /* World display status             */
extern  int     splatox;                /* Display splatted ox              */
extern  int     oxsplatted;             /* An ox has been splotted          */
extern  int     movetick, movemax;      /* Move timing                      */

extern  int     dispcomp(),             /*  Display and move functions      */
                movecomp(),
                dispplyr(),
                moveplyr(),
                moveshot(),
                dispbomb(),
                movebomb(),
                movetarg(),
                disptarg(),
                moveexpl(),
                dispexpl(),
                movesmok(),
                movebird(),
                dispbird(),
                moveflck(),
                dispflck(),
                moveox(),
                movemiss(),
                dispmiss(),
                moveburst(),
                dispburst();

extern  OBJECTS *allocobj();

extern  int      swbreak(), swtick(), swshfprt(), swkeyint();
extern  int      counttick, countmove;   /* Performance counters   */

extern  jmp_buf envrestart;             /* Restart environment for restart  */
                                        /*  long jump.                      */

extern  int     endsts[];               /* End of game status and move count*/
extern  int     endcount;
extern  BOOL    goingsun;               /* Heading for the sun flag         */
extern  OBJECTS *compnear[];            /*  Array of planes near computers*/
extern  unsigned explseed;              /* explosion seed                 */

extern  char    *multfile;              /* Multi user files               */
extern  char    *cmndfile;
extern  unsigned multaddr;              /* Multiple user diskette adapter */
                                        /*   address                      */

extern  int      maxcrash;              /* Maximum number of crashes      */

extern  int      ctlbflag;              /* Control break has been pressed */

static  int      savescore;             /* save players score on restart  */
static  BOOL     ghost;                 /* ghost display flag             */

static  char     *helptxt[] = {         /* Help text                      */
	"",
	"SOPWITH, Distribution Version",
	"(c) Copyright 1984-2000 David L. Clark",
	"Modification Date:  October 29, 2000",
	"",
	"Usage:  sopwith [options]",
	"The options are:",
	"        -n :  novice single player",
	"        -s :  single player",
	"        -c :  single player against computer",
/*
  "        -m :  multiple players on a network",
*/
	"        -a :  2 players over asynchrounous communications line",
	"              (Only one of -n, -s, -c, -a may be specified)",
	"        -k :  keyboard only",
	"        -j :  joystick and keyboard",
	"              (Only one of -k and -j  may be specified)",
/*
  "        -i :  IBM PC keyboard",
*/
	"        -q :  begin game with sound off",
/*
  "        -r :  resets the multiuser communications file after an abnormal",
  "                  end of a game",
  "        -d*:  overrides the default drive C as the device containing the",
  "                  communications file",
*/
	"        -p#:  overrides asynchronous port 1 as the asynchrounous port",
	"                  to use",
	NULL
};

// sdh: not much changed here, some interrupt stuff commented out

swinit( argc, argv )
int     argc;
char    *argv[];
{
BOOL    reset = FALSE;
BOOL    n = FALSE;
BOOL    s = FALSE;
BOOL    c = FALSE;
BOOL    m = FALSE;
BOOL    a = FALSE;
BOOL    k = FALSE;
int     modeset, keyset;
char    *device   = "\0              ";


        if ( getflags( &argc, &argv,
                        /*---- 96/12/27------
                        "n&s&c&m&a&k&i&j&q&h*v*r&d*f*n*t#w&y#e&g#x&:",
                        &n, &s, &c, &m, &a, &k, &ibmkeybd, &joystick, &soundflg,
                        &histout, &histin, &reset, &device,
                        &multfile, &cmndfile, &multtick, &hires, &keydelay,
                        &repflag, &gamenum, &missok )
                        ------ 96/12/27----*/
                        /*---- 99/01/24------
                        "n&s&c&m&a&k&j&q&h*v*r&d*f*t#w&y#e&g#x&:",
                        &n, &s, &c, &m, &a, &k, &joystick, &soundflg,
                        &histout, &histin, &reset, &device,
                        &multfile, &multtick, &hires, &keydelay,
                        &repflag, &gamenum, &missok )
                        ------ 99/01/24----*/
                        "n&s&c&a&k&j&q&x&:",
                        &n, &s, &c, &a, &k, &joystick, &soundflg,
                        &missok )
                || ( ( modeset = n + s + c + m + a ) > 1 )
                || ( ( keyset = joystick + k ) > 1 ) ) {
                disphelp( helptxt );
                exit( 1 );
        }

        soundflg = !soundflg;
        if ( modeset && keyset )
                titleflg = TRUE;

        movemax=15;
        initseed();

	CGA_Init();             // init CGA driver
	Speaker_Init();         // init pc speaker
	
	//        explseed = histinit( explseed );
        initsndt();
        intsoff();
//        _intsetup( BREAKINT, swbreak, csseg(), dsseg() );
//        _intsetup( CLOCKINT, swtick, csseg(), dsseg() );
        initgrnd();
        swtitln();
        intson();

	
        if ( modeset )
                playmode = n ? NOVICE
                             : ( s ? SINGLE
                             : ( c ? COMPUTER
                             : ( m ? MULTIPLE : ASYNCH ) ) );
        else
                getmode();
	
//        if ( !keyset )
//                getkey();

        if ( ( playmode == MULTIPLE ) || ( playmode == ASYNCH ) ) {
                maxcrash = MAXCRASH * 2;
                if ( playmode == MULTIPLE )
                        init1mul( reset, device );
                else
                        init1asy();
                initgrnd();
                initobjs();
                if ( playmode == MULTIPLE )
                        init2mul();
                else
                        init2asy();
                inittarg();
                if ( currgame->gm_specf )
                        ( *currgame->gm_specf ) ();
                initdisp( NO );
                if ( keydelay == -1 )
                        keydelay = 1;
        } else {
                if ( keydelay == -1 )
                        keydelay = 1;
                maxcrash = MAXCRASH;
                currgame = &swgames[0];
                clrprmpt();
                initobjs();
                initplyr( NULL );
                initcomp( NULL );
                initcomp( NULL );
                initcomp( NULL );
                inittarg();
                if ( currgame->gm_specf )
                        ( *currgame->gm_specf ) ();
                initdisp( NO );
        }

        initflck();
        initoxen();

        initgdep();

        intsoff();
//        _intsetup( PRINTINT, swshfprt, csseg(), dsseg() );
//        koveride = _intsetup( KEYINT, swkeyint, csseg(), dsseg() );
        inplay = TRUE;
        intson();
}



static  disphelp( hp )
char    **hp;
{
register char   **h;

        h = hp;
        puts( "\r\n" );
        while ( *h ) {
                puts( *h++ );
                puts( "\r\n" );
        }
}





initseed()
{
#ifdef IBMPC
        while ( !explseed ) {
                outportb( 0x43, 0 );
                explseed = ( 0x00FF & inportb( 0x40 ) )
                         | ( 0xFF00 & ( inportb( 0x40 ) << 8 ) );
        }
#endif
#ifdef ATARI
long    trap14();

        explseed = trap14( 17 );
#endif
}





swrestart()
{
register OBJECTS  *ob;
register int      tickwait, inc;

        if ( endsts[player] == WINNER ) {
                ob = &nobjects[player];
                inc = 0;
                while ( ob->ob_crashcnt++ < maxcrash ) {
                        ob->ob_score += ( inc += 25 );
                        setvdisp();
                        dispcgge( ob );
                        dispscore( ob );
                        intsoff();
                        tickwait = 5;
                        counttick = 0;
                        intson();
                        //while ( counttick < tickwait );
                }
                ++gamenum;
                savescore = ob->ob_score;
        } else {
                gamenum = 0;
                savescore = 0;
        }

        initsndt();
        initgrnd();
        initobjs();
        initplyr( NULL );
        initcomp( NULL );
        initcomp( NULL );
        initcomp( NULL );
        inittarg();
        if ( currgame->gm_specf )
                ( *currgame->gm_specf ) ();

        initdisp( NO );
        initflck();
        initoxen();
        initgdep();

        longjmp( envrestart, 0 );
}



initgdep()
{

        gmaxspeed = MAX_SPEED + gamenum;
        gminspeed = MIN_SPEED + gamenum;

        targrnge = 150;
        if ( gamenum < 6 )
                targrnge -= 15 * ( 6 - gamenum );
        targrnge *= targrnge;
}






clrprmpt()
{
	// sdh: generic clear

	int x, y;

	for(y=0; y<32; ++y)
		for(x=0; x<SCR_WDTH; ++x) {
			swpntsym(x, SCR_HGHT-(y+160), 0);
		}

        swposcur( 0, 20 );
}




static getkey()
{
        register char    key;
 
        /*----------------97/12/27--------------
        clrprmpt();
        swputs( "Key: 1 - Joystick with IBM Keyboard\r\n" );
        swputs( "     2 - Joystick with non-IBM Keyboard\r\n" );
        swputs( "     3 - IBM Keyboard only\r\n" );
        swputs( "     4 - Non-IBM keyboard only\r\n" );
        FOREVER {
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( ( key = swgetc() & 0x00FF ) < '1' )
                        || ( key > '4' ) )
                        continue;
                joystick = ( key <= '2' );
                ibmkeybd = ( key == '1' ) || ( key == '3' );
                return;
        }
        ------------------97/12/27--------------*/
        clrprmpt();
        swputs( "Key: K - Keyboard Only\r\n" );
        swputs( "     J - Joystick and Keyboard\r\n" );

	CGA_Update();
	
	FOREVER {
		swsndupdate();
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( ( key = toupper(swgetc() & 0x00FF) ) != 'K' )
                        && ( key != 'J' ) )
                        continue;
                joystick = key == 'J';
                ibmkeybd = 1;
                return;
        }
}





static getmode()
{
        clrprmpt();
        swputs( "Key: S - single player\r\n" );
        swputs( "     C - single player against computer\r\n" );

	CGA_Update();
	
	FOREVER {
		char c;

		swsndupdate();
                if ( ctlbreak() )
                        swend( NULL, NO );

		c = toupper(swgetc() & 0xff);
		
                switch ( c ) {
                        case 'S':
                                clrprmpt();
                                swputs( "Key: N - novice player\r\n" );
                                swputs( "     E - expert player\r\n" );

				CGA_Update();
				
				FOREVER {
					swsndupdate();
                                        if ( ctlbreak() )
                                                swend( NULL, NO );
                                        switch ( toupper( swgetc()
                                                        & 0x00FF ) ){
                                                case 'N':
                                                        playmode = NOVICE;
                                                        return;
                                                case 'E':
                                                        playmode = SINGLE;
                                                        return;
                                        }
                                }
                        case 'M':
                                /*----- 2000/10/29 ----------
                                if ( _systype == PCDOS ) {
                                        playmode = MULTIPLE;
                                        return;
                                }
                                ------- 2000/10/29 --------*/
                                break;
                        case 'C':
                                playmode = COMPUTER;
                                return;
                        case 'A':
                                if ( _systype == PCDOS ) {
                                        playmode = ASYNCH;
                                        return;
                                }
                                break;
                        default:
                                break;
				
                }
        }
}




getgame()
{
        register int     game;

        clrprmpt();
        swputs( "         Key a game number" );
        FOREVER {
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( ( game = ( swgetc() & 0x00FF ) - '0' ) >= 0 )
                    && ( game <= MAX_GAME ) )
                        return( game );
        }
}




BOOL    ctlbreak()
{
        return( ctlbflag );
}




static  initgrnd()
{
        movmem( orground, ground, sizeof( GRNDTYPE ) * MAX_X );
}




initdisp( reset )
BOOL    reset;
{
	register OBJECTS *ob;
	OBJECTS          ghostob;
	extern   char    swghtsym[];

        splatox = oxsplatted = 0;
        if ( !reset ) {
                clrdispa();
                setadisp();
                dispworld();
                swtitlf();
                ghost = FALSE;
        }
        movedisp();
        setvdisp();
        initwobj();
        initscore();

        ob = &nobjects[player];
        if ( ghost ) {
                ghostob.ob_type = DUMMYTYPE;
                ghostob.ob_symhgt = ghostob.ob_symwdt = 8;
                ghostob.ob_clr = ob->ob_clr;
                ghostob.ob_newsym = swghtsym;
                swputsym( GHOSTX, 12, &ghostob );
        } else {
                dispfgge( ob );
                dispbgge( ob );
                dispmgge( ob );
                dispsbgge( ob );
                dispsgge( ob );
                dispcgge( ob );
        }
        dispinit = TRUE;
}



static  initscore()
{
        if ( savescore ) {
                  nobjects[0].ob_score = savescore;
                  savescore = 0;
        }

        dispscore( &nobjects[0] );
        if ( ( ( playmode == MULTIPLE ) || ( playmode == ASYNCH ) )
                && ( multbuff->mu_maxplyr > 1 ) )
                dispscore( &nobjects[1] );
}




dispcgge( ob )
OBJECTS *ob;
{
        dispgge( CGUAGEX, maxcrash - ob->ob_crashcnt, maxcrash, ob->ob_clr );
}




dispfgge( ob )
OBJECTS *ob;
{
        dispgge( FGUAGEX, ob->ob_life >> 4, MAXFUEL >> 4, ob->ob_clr );
}




dispbgge( ob )
OBJECTS *ob;
{
        dispgge( BGUAGEX, ob->ob_bombs, MAXBOMBS, 3 - ob->ob_clr );
}




dispsgge( ob )
OBJECTS *ob;
{
        dispgge( SGUAGEX, ob->ob_rounds, MAXROUNDS, 3 );
}




dispmgge( ob )
OBJECTS *ob;
{
        dispgge( MGUAGEX, ob->ob_missiles, MAXMISSILES, ob->ob_clr );
}




dispsbgge( ob )
OBJECTS *ob;
{
        dispgge( SBGUAGEX, ob->ob_bursts, MAXBURSTS, 3 - ob->ob_clr );
}




static  dispgge( x, cury, maxy, clr )
int     x, cury, maxy, clr;
{
register int     y;

        if ( ghost )
                return;

        cury = cury * 10 / maxy - 1;
        if ( cury > 9 )
                cury = 9;
        for ( y = 0; y <= cury; ++y )
                swpntsym( x, y, clr );
        for ( ; y <= 9; ++y )
                swpntsym( x, y, 0 );
}



static dispworld()
{
register int     x, y, dx, maxh, sx;

        dx = 0;
        sx = SCR_CENTR;

        maxh = 0;
        y = 0;
        for ( x = 0; x < MAX_X; ++x ) {

                if ( ground[x] > maxh )
                        maxh = ground[x];

                if ( ( ++dx ) == WRLD_RSX ) {
                        maxh /= WRLD_RSY;
                        if ( maxh == y )
                                swpntsym( sx, maxh, 7 );
                        else
                                if ( maxh > y )
                                        for ( ++y; y <= maxh; ++y )
                                                swpntsym( sx, y, 7 );
                                else
                                        for ( --y; y >= maxh; --y )
                                                swpntsym( sx, y, 7 );
                        y = maxh;
                        swpntsym( sx, 0, 11 );
                        ++sx;
                        dx = maxh = 0;
                }
        }
        maxh = MAX_Y / WRLD_RSY;
        for ( y = 0; y <= maxh; ++y ) {
                swpntsym( SCR_CENTR, y, 11 );
                swpntsym( sx, y, 11 );
        }

        for ( x = 0; x < SCR_WDTH; ++x )
                swpntsym( x, (SCR_MNSH+2), 7 );

	CGA_Update();
}





static initwobj()
{
int     x;
register OBJECTS *ob;
register OLDWDISP *ow;

        ow = wdisp;
        ob = nobjects;
        for ( x = 0; x < MAX_OBJS; ++x, ow++, ob++ )
                ow->ow_xorplot = ob->ob_drwflg = ob->ob_delflg = 0;

        for ( x = 0; x < MAX_TARG; ++x )
                if ( ( ob = targets[x] ) && ( ob->ob_state != FINISHED ) )
                        dispwobj( ob );

}



static movedisp()
{
#ifdef IBMPC
        swsetblk( 0,        SCR_SEGM, 0x1000, 0 );
        swsetblk( SCR_ROFF, SCR_SEGM, 0x1000, 0 );
        movblock( auxdisp,          dsseg(), 0x1000, SCR_SEGM, 0x1000 );
        movblock( auxdisp + 0x1000, dsseg(), 0x3000, SCR_SEGM, 0x1000 );
#endif

#ifdef ATARI
long    trap14(), vidram;

        vidram = trap14( 3 );
        setmem( vidram, 0x4000, 0 );
        movmem( auxdisp, vidram + 0x4000, 0x4000 );
#endif
        int x, y;

        for(y=20; y<SCR_HGHT; ++y)
                for(x=0; x<SCR_WDTH; ++x)
                        swpntsym(x, y, 0);

//	CGA_ClearScreen();
	dispworld();
}




static clrdispa()
{
	CGA_ClearScreen();
}




static initobjs()
{
register OBJECTS *ob;
register int     o;

        topobj.ob_xnext = topobj.ob_next = &botobj;
        botobj.ob_xprev = botobj.ob_prev = &topobj;
        topobj.ob_x = -32767;
        botobj.ob_x = 32767;

        objbot = objtop = deltop = delbot = NULL;
        objfree = ob = nobjects;

        for ( o = 0; o < MAX_OBJS; ++o ) {
                ob->ob_next = ob + 1;
                (ob++)->ob_index = o;
        }

        (--ob)->ob_next = NULL;

}



initcomp( obp )
OBJECTS *obp;
{
register OBJECTS *ob;
OBJECTS          *initpln();


        ob = initpln( obp );
        if ( !obp ) {
                ob->ob_drawf = dispcomp;
                ob->ob_movef = movecomp;
                ob->ob_clr = 2;
                if ( ( playmode != MULTIPLE ) && ( playmode != ASYNCH ) )
                        ob->ob_owner = &nobjects[1];
                else
                        if ( ob->ob_index == 1 )
                                ob->ob_owner = ob;
                        else
                                ob->ob_owner = ob - 2;
                movmem( ob, &oobjects[ob->ob_index], sizeof( OBJECTS ) );
        }
        if ( ( playmode == SINGLE ) || ( playmode == NOVICE ) ){
                 ob->ob_state = FINISHED;
                 deletex( ob );
        }
}



initplyr( obp )
OBJECTS *obp;
{
	register OBJECTS *ob;
	OBJECTS          *initpln();

        ob = initpln( obp );
        if ( !obp ) {
                ob->ob_drawf = dispplyr;
                ob->ob_movef = moveplyr;
                ob->ob_clr = ob->ob_index % 2 + 1;
                ob->ob_owner = ob;
                movmem( ob, &oobjects[ob->ob_index], sizeof( OBJECTS ) );
                goingsun = FALSE;
                endcount = 0;
        }

        displx = ob->ob_x - SCR_CENTR;
        disprx = displx + SCR_WDTH - 1;

        swflush();
}



static  int     inits[2] = { 0, 7 };
static  int     initc[4] = { 0, 7, 1, 6 };
static  int     initm[8] = { 0, 7, 3, 4, 2, 5, 1, 6 };



OBJECTS *initpln( obp )
OBJECTS *obp;
{
register OBJECTS *ob;
register int     x, height, minx, maxx, n;

        if ( !obp )
                ob = allocobj();
        else
                ob = obp;

        switch ( playmode ) {
                case SINGLE:
                case NOVICE:
                        n = inits[ob->ob_index];
                        break;
                case MULTIPLE:
                case ASYNCH:
                        n = initm[ob->ob_index];
                        break;
                case COMPUTER:
                        n = initc[ob->ob_index];
                        break;
        }

        ob->ob_type = PLANE;

        ob->ob_x = currgame->gm_x[n];
        minx = ob->ob_x;
        maxx = ob->ob_x + 20;
        height = 0;
        for ( x = minx; x <= maxx; ++x )
                if ( ground[x] > height )
                        height = ground[x];
        ob->ob_y = height + 13;
        ob->ob_lx = ob->ob_ly = ob->ob_speed = ob->ob_flaps = ob->ob_accel
                  = ob->ob_hitcount = ob->ob_bdelay = ob->ob_mdelay
                  = ob->ob_bsdelay = 0;
        setdxdy( ob, 0, 0 );
        ob->ob_orient = currgame->gm_orient[n];
        ob->ob_angle = ( ob->ob_orient ) ? ( ANGLES / 2 ) : 0;
        ob->ob_target = ob->ob_firing = ob->ob_mfiring = NULL;
        ob->ob_bombing = ob->ob_bfiring = ob->ob_home = FALSE;
        ob->ob_symhgt = SYM_HGHT;
        ob->ob_symwdt = SYM_WDTH;
        ob->ob_athome = TRUE;
        if ( ( !obp ) || ( ob->ob_state == CRASHED )
                || ( ob->ob_state == GHOSTCRASHED ) ) {
                ob->ob_rounds = MAXROUNDS;
                ob->ob_bombs = MAXBOMBS;
                ob->ob_missiles = MAXMISSILES;
                ob->ob_bursts = MAXBURSTS;
                ob->ob_life = MAXFUEL;
        }
        if ( !obp ) {
                ob->ob_score = ob->ob_updcount = ob->ob_crashcnt
                             = endsts[ob->ob_index] = 0;
                compnear[ob->ob_index] = NULL;
                insertx( ob, &topobj );
        } else {
                deletex( ob );
                insertx( ob, ob->ob_xnext );
        }

        if ( ( ( playmode == MULTIPLE ) || ( playmode == ASYNCH ) )
                && ( ob->ob_crashcnt >= maxcrash ) ) {
                ob->ob_state = GHOST;
                if ( ob->ob_index == player )
                        ghost = TRUE;
        } else
                ob->ob_state = FLYING;

        return( ob );
}



initshot( obop, targ )
OBJECTS *obop, *targ;
{
register OBJECTS *ob, *obo;
int      nangle, nspeed, dx, dy, r, bspeed, x, y;


        if ( ( !targ ) && ( !compplane ) && ( !obo->ob_rounds ))
                return;

        if ( !( ob = allocobj() ) )
                return;

        obo = obop;
        if ( playmode != NOVICE )
                --obo->ob_rounds;

        bspeed = BULSPEED + gamenum;

        if ( targ ) {
                x = targ->ob_x + ( targ->ob_dx << 2 );
                y = targ->ob_y + ( targ->ob_dy << 2 );
                dx = x - obo->ob_x;
                dy = y - obo->ob_y;
                if ( ( r = isrange( x, y, obo->ob_x, obo->ob_y ) ) < 1 ) {
                        deallobj( ob );
                        return;
                }
                ob->ob_dx = ( dx * bspeed ) / r;
                ob->ob_dy = ( dy * bspeed ) / r;
                ob->ob_ldx = ob->ob_ldy = 0;
        } else {
                nspeed = obo->ob_speed + bspeed;
                nangle = obo->ob_angle;
                setdxdy( ob, nspeed * COS( nangle ),
                             nspeed * SIN( nangle ) );
        }

        ob->ob_type = SHOT;
        ob->ob_x = obo->ob_x + SYM_WDTH / 2;
        ob->ob_y = obo->ob_y - SYM_HGHT / 2;
        ob->ob_lx = obo->ob_lx;
        ob->ob_ly = obo->ob_ly;


        ob->ob_life = BULLIFE;
        ob->ob_owner = obo;
        ob->ob_clr = obo->ob_clr;
        ob->ob_symhgt = ob->ob_symwdt = 1;
        ob->ob_drawf = NULL;
        ob->ob_movef = moveshot;
        ob->ob_speed = 0;

        insertx( ob, obo );

}





static isrange( x, y, ax, ay )
int     x, y, ax, ay;
{
register int     dx, dy, t;

        dy = abs( y - ay );
        dy += dy >> 1;
        if ( ( ( dx = abs( x - ax ) ) > 100 ) || ( dy > 100 ) )
                return ( -1 );

        if ( dx < dy ) {
                t = dx;
                dx = dy;
                dy = t;
        }

        return( ( ( 7 * dx ) + ( dy << 2 ) ) >> 3 );
}



initbomb( obop )
OBJECTS *obop;
{
register OBJECTS *ob, *obo;
int              angle;

        obo = obop;
        if ( ( ( !compplane) && ( !obo->ob_bombs ) ) || ( obo->ob_bdelay ) )
                return;
        if ( !( ob = allocobj() ) )
                return;

        if ( playmode != NOVICE )
                --obo->ob_bombs;

        obo->ob_bdelay = 10;

        ob->ob_type = BOMB;
        ob->ob_state = FALLING;
        ob->ob_dx = obo->ob_dx;
        ob->ob_dy = obo->ob_dy;

        if ( obo->ob_orient )
                angle = ( obo->ob_angle + ( ANGLES / 4 ) ) % ANGLES;
        else
                angle = ( obo->ob_angle + ( 3 * ANGLES / 4 ) ) % ANGLES;
        ob->ob_x = obo->ob_x + ( ( COS( angle ) * 10 ) >> 8 ) + 4;
        ob->ob_y = obo->ob_y + ( ( SIN( angle ) * 10 ) >> 8 ) - 4;
        ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;

        ob->ob_life = BOMBLIFE;
        ob->ob_owner = obo;
        ob->ob_clr = obo->ob_clr;
        ob->ob_symhgt = ob->ob_symwdt = 8;
        ob->ob_drawf = dispbomb;
        ob->ob_movef = movebomb;

        insertx( ob, obo );

}



initmiss( obop )
OBJECTS *obop;
{
register OBJECTS *ob, *obo;
int              angle, nspeed;

        obo = obop;
        if ( obo->ob_mdelay || ( !obo->ob_missiles ) || !missok )
                return;
        if ( !( ob = allocobj() ) )
                return;

        if ( playmode != NOVICE )
                --obo->ob_missiles;

        obo->ob_mdelay = 5;

        ob->ob_type = MISSILE;
        ob->ob_state = FLYING;

        angle = ob->ob_angle = obo->ob_angle;
        ob->ob_x = obo->ob_x + ( COS( angle ) >> 4 ) + 4;
        ob->ob_y = obo->ob_y + ( SIN( angle ) >> 4 ) - 4;
        ob->ob_lx = ob->ob_ly = 0;
        ob->ob_speed = nspeed = gmaxspeed + ( gmaxspeed >> 1 );
        setdxdy( ob, nspeed * COS( angle ), nspeed * SIN( angle ) );

        ob->ob_life = MISSLIFE;
        ob->ob_owner = obo;
        ob->ob_clr = obo->ob_clr;
        ob->ob_symhgt = ob->ob_symwdt = 8;
        ob->ob_drawf = dispmiss;
        ob->ob_movef = movemiss;
        ob->ob_target = obo->ob_mfiring;
        ob->ob_orient = ob->ob_accel = ob->ob_flaps = 0;

        insertx( ob, obo );

}



initburst( obop )
OBJECTS *obop;
{
register OBJECTS *ob, *obo;
int              angle;

        obo = obop;
        if ( obo->ob_bsdelay || ( !obo->ob_bursts ) || !missok )
                return;
        if ( !( ob = allocobj() ) )
                return;

        ob->ob_bsdelay = 5;

        if ( playmode != NOVICE )
                --obo->ob_bursts;

        ob->ob_type = STARBURST;
        ob->ob_state = FALLING;

        if ( obo->ob_orient )
                angle = ( obo->ob_angle + ( 3 * ANGLES / 8 ) ) % ANGLES;
        else
                angle = ( obo->ob_angle + ( 5 * ANGLES / 8 ) ) % ANGLES;
        setdxdy( ob, gminspeed * COS( angle ), gminspeed * SIN( angle ) );
        ob->ob_dx += obo->ob_dx;
        ob->ob_dy += obo->ob_dy;

        ob->ob_x = obo->ob_x + ( ( COS( angle ) * 10 ) >> 10 ) + 4;
        ob->ob_y = obo->ob_y + ( ( SIN( angle ) * 10 ) >> 10 ) - 4;
        ob->ob_lx = ob->ob_ly = 0;

        ob->ob_life = BURSTLIFE;
        ob->ob_owner = obo;
        ob->ob_clr = obo->ob_clr;
        ob->ob_symhgt = ob->ob_symwdt = 8;
        ob->ob_drawf = dispburst;
        ob->ob_movef = moveburst;

        insertx( ob, obo );

}



static inittarg()
{
register OBJECTS *ob;
register int x, i;
int     *tx, *tt;
int     minh, maxh, aveh, minx, maxx;

        tx = currgame->gm_xtarg;
        tt = currgame->gm_ttarg;

        if ( ( ( playmode != MULTIPLE ) && ( playmode != ASYNCH ) )
                || ( multbuff->mu_maxplyr == 1 ) ) {
                numtarg[0] = 0;
                numtarg[1] = MAX_TARG - 3;
        } else
                numtarg[0] = numtarg[1] = MAX_TARG / 2;

        for ( i = 0; i < MAX_TARG; ++i, ++tx, ++tt ) {
                targets[i] = ob = allocobj();
                minx = ob->ob_x = *tx;
                maxx = ob->ob_x + 15;
                minh = 999;
                maxh = 0;
                for ( x = minx; x <= maxx; ++x ) {
                        if ( ground[x] > maxh )
                                maxh = ground[x];
                        if ( ground[x] < minh )
                                minh = ground[x];
                }
                aveh = ( minh + maxh ) >> 1;

                while ( ( ob->ob_y = aveh + 16 ) >= MAX_Y )
                        --aveh;

                for ( x = minx; x <= maxx; ++x )
                        ground[x] = aveh;

                ob->ob_dx = ob->ob_dy = ob->ob_lx = ob->ob_ly = ob->ob_ldx
                          = ob->ob_ldy = ob->ob_angle = ob->ob_hitcount = 0;
                ob->ob_type = TARGET;
                ob->ob_state = STANDING;
                ob->ob_orient = *tt;
                ob->ob_life = i;

                if ( ( ( playmode != MULTIPLE ) && ( playmode != ASYNCH ) )
                        || ( multbuff->mu_maxplyr == 1 ) )
                        ob->ob_owner = &nobjects[( ( i < MAX_TARG / 2 )
                                                 && ( i > MAX_TARG /2 - 4 ) )
                                                 ? 0 : 1 ];
                else
                        ob->ob_owner = &nobjects[i >= ( MAX_TARG / 2 ) ];
                ob->ob_clr = ob->ob_owner->ob_clr;
                ob->ob_symhgt = ob->ob_symwdt = 16;
                ob->ob_drawf = disptarg;
                ob->ob_movef = movetarg;

                insertx( ob, &topobj );
        }
}



initexpl( obop, small )
OBJECTS *obop;
int     small;
{
register OBJECTS *ob, *obo;
int      i, ic, life, speed;
int      obox, oboy, obodx, obody, oboclr, obotype;
BOOL     mansym;
int      orient;


        obo = obop;
        obox   = obo->ob_x + ( obo->ob_symwdt >> 1 );
        oboy   = obo->ob_y + ( obo->ob_symhgt >> 1 );
        obodx  = obo->ob_dx >> 2;
        obody  = obo->ob_dy >> 2;
        oboclr = obo->ob_clr;

        if ( ( ( obotype = obo->ob_type ) == TARGET )
                && ( obo->ob_orient == 2 ) ) {
                ic = 1;
                speed = gminspeed;
        } else {
                ic = small ? 6 : 2;
                speed = gminspeed >> ( ( explseed & 7 ) != 7 );
        }
        mansym = ( obotype == PLANE ) && ( ( obo->ob_state == FLYING )
                                      || ( obo->ob_state == WOUNDED ) );

        for ( i = 1; i <= 15; i += ic ) {
                if ( !( ob = allocobj() ) )
                        return;

                ob->ob_type = EXPLOSION;

                setdxdy( ob, COS( i ) * speed, SIN( i ) * speed );
                ob->ob_dx += obodx;
                ob->ob_dy += obody;

                if ( !( explseed = ( ob->ob_x = obox + ob->ob_dx )
                        * ( ob->ob_y = oboy + ob->ob_dy )
                        * explseed + 7491 ) )
                        explseed = 74917777;

                ob->ob_life = EXPLLIFE;
                orient = ob->ob_orient =   ( explseed & 0x01C0 ) >> 6;
                if ( mansym && ( ( !orient ) || ( orient == 7 ) ) ) {
                        mansym = orient = ob->ob_orient = 0;
                        ob->ob_dx = obodx;
                        ob->ob_dy = -gminspeed;
                }

                ob->ob_lx = ob->ob_ly = ob->ob_hitcount = ob->ob_speed = 0;
                ob->ob_owner = obo;
                ob->ob_clr = oboclr;
                ob->ob_symhgt = ob->ob_symwdt = 8;
                ob->ob_drawf = dispexpl;
                ob->ob_movef = moveexpl;

                if ( orient )
                        initsound( ob, S_EXPLOSION );

                insertx( ob, obo );
        }
}



initsmok( obop )
OBJECTS *obop;
{
register OBJECTS *ob, *obo;


        if ( !( ob = allocobj() ) )
                return;

        ob->ob_type = SMOKE;

        ob->ob_x = ( obo = obop )->ob_x + 8;
        ob->ob_y = obo->ob_y - 8;
        ob->ob_dx = obo->ob_dx;
        ob->ob_dy = obo->ob_dy;
        ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;
        ob->ob_life = SMOKELIFE;
        ob->ob_owner = obo;
        ob->ob_symhgt = ob->ob_symwdt = 1;
        ob->ob_drawf = NULL;
        ob->ob_movef = movesmok;
        ob->ob_clr = obo->ob_clr;
}




static  ifx[] = { MINFLCKX, MINFLCKX + 1000, MAXFLCKX - 1000, MAXFLCKX };
static  ify[] = { MAX_Y-1,  MAX_Y-1,         MAX_Y-1,         MAX_Y-1  };
static  ifdx[] = { 2,       2,               -2,              -2       };



initflck()
{
register OBJECTS *ob;
register int     i, j;

        if ( playmode == NOVICE )
                return;

        for ( i = 0; i < MAX_FLCK; ++i ) {

                if ( !( ob = allocobj() ) )
                        return;

                ob->ob_type = FLOCK;
                ob->ob_state = FLYING;
                ob->ob_x = ifx[i];
                ob->ob_y = ify[i];
                ob->ob_dx = ifdx[i];
                ob->ob_dy = ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;
                ob->ob_orient = 0;
                ob->ob_life = FLOCKLIFE;
                ob->ob_owner = ob;
                ob->ob_symhgt = ob->ob_symwdt = 16;
                ob->ob_drawf = dispflck;
                ob->ob_movef = moveflck;
                ob->ob_clr = 9;
                insertx( ob, &topobj );
                for ( j = 0; j < MAX_BIRD; ++j )
                        initbird( ob, 1 );
        }
}



initbird( obop, i )
OBJECTS *obop;
int     i;
{
register OBJECTS *ob, *obo;
static   ibx[] = { 8, 3, 0, 6, 7, 14, 10, 12 };
static   iby[] = { 16, 1, 8, 3, 12, 10, 7, 14 };
static   ibdx[] = { -2, 2, -3, 3, -1, 1, 0, 0 };
static   ibdy[] = { -1, -2, -1, -2, -1, -2, -1, -2 };


        if ( !( ob = allocobj() ) )
                return;

        ob->ob_type = BIRD;

        ob->ob_x = ( obo = obop )->ob_x + ibx[i];
        ob->ob_y = obo->ob_y - iby[i];
        ob->ob_dx = ibdx[i];
        ob->ob_dy = ibdy[i];
        ob->ob_orient = ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy = 0;
        ob->ob_life = BIRDLIFE;
        ob->ob_owner = obo;
        ob->ob_symhgt = 2;
        ob->ob_symwdt = 4;
        ob->ob_drawf = dispbird;
        ob->ob_movef = movebird;
        ob->ob_clr = obo->ob_clr;
        insertx( ob, obo );
}



initoxen()
{
register OBJECTS *ob;
register int      i;
static   iox[] = { 1376, 1608 };
static   ioy[] = { 80,   91   };

        if ( playmode == NOVICE ) {
                for ( i = 0; i < MAX_OXEN; ++i )
                        targets[MAX_TARG + i] = NULL;
                return;
        }

        for ( i = 0; i < MAX_OXEN; ++i ) {
                if ( !( targets[MAX_TARG + i] = ob = allocobj() ) )
                        return;

                ob->ob_type = OX;
                ob->ob_state = STANDING;
                ob->ob_x = iox[i];
                ob->ob_y = ioy[i];
                ob->ob_orient = ob->ob_lx = ob->ob_ly = ob->ob_ldx = ob->ob_ldy
                              = ob->ob_dx = ob->ob_dy = 0;
                ob->ob_owner = ob;
                ob->ob_symhgt = 16;
                ob->ob_symwdt = 16;
                ob->ob_drawf = NULL;
                ob->ob_movef = moveox;
                ob->ob_clr = 1;
                insertx( ob, &topobj );
        }
}

