/*
        swmain   -      SW mainline

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-02        Development
                        84-06-12        PC-jr Speed-up
                        85-10-31        Atari
                        87-03-09        Microsoft compiler.
                        87-03-12        Wounded airplanes.
                        87-04-06        Computer plane avoiding oxen.
                        96-12-26        Speed up game a bit
*/

#include <stdio.h>
#include "sw.h"
#include "timer.h"

// sdh: framerate control

#define FPS 10

int     playmode;                       /* Mode of play                     */
GAMES   *currgame;                      /* Game parameters and current game */
OBJECTS *targets[MAX_TARG+MAX_OXEN];    /* Status of targets array          */
int     numtarg[2];                     /* Number of active targets by color*/
int     savemode;                       /* Saved PC display mode            */
int     tickmode;                       /* Tick action to be performed      */
int     counttick, countmove;           /* Performance counters             */
int     movetick, movemax;              /* Move timing                      */

int     gamenum;                        /* Current game number              */
int     gmaxspeed, gminspeed;           /* Speed range based on game number */
int     targrnge;                       /* Target range based on game number*/

MULTIO  multiost;                       /* Multiple player I/O buffer       */

char    auxdisp[0x2000];                /* Auxiliary display area           */

#ifdef  ATARI
char    auxdisp[0x4000];
#endif

int     multkey;                        /* Keystroke to be passed           */
MULTIO  *multbuff = &multiost;

#ifdef  IBMPC
unsigned multaddr = 0x3F2;              /* Multiple user diskette adapter   */
#endif                                  /*   address                        */
#ifdef  ATARI
unsigned multaddr = 0;                  /* Multiple user controller number  */
#endif

int     multtick;                       /* Multiple user tick delay         */
BOOL    hires;                          /* High res flag                    */
BOOL    disppos;                        /* Display position flag            */
BOOL    titleflg;                       /* Title flag                       */
int     dispdbg;                        /* Debug value to display           */
BOOL    soundflg;                       /* Sound flag                       */
BOOL    repflag;                        /* Report statistics flag           */
BOOL    joystick;                       /* Joystick being used              */
BOOL    ibmkeybd;                       /* IBM-like keyboard being used     */
BOOL    inplay;                         /* Game is in play                  */
BOOL    printflg = 0;                   /* Print screen requested           */
int     koveride;                       /* Keyboard override index number   */
int     missok;                         /* Missiles supported               */

int     displx, disprx;                 /* Display left and right           */
int     dispdx;                         /* Display shift                    */
BOOL    dispinit;                       /* Inialized display flag           */

OBJECTS *drawlist;                      /* Onscreen object list             */
OBJECTS *nobjects;                      /* Objects list.                    */
OBJECTS oobjects[MAX_PLYR];             /* Original plane object description*/
OBJECTS *objbot, *objtop,               /* Top and bottom of object list    */
        *objfree,                       /* Free list                        */
        *deltop, *delbot;               /* Newly deallocated objects        */
OBJECTS topobj, botobj;                 /* Top and Bottom of obj. x list    */

OBJECTS *compnear[MAX_PLYR];            /* Planes near computer planes      */
int     lcompter[MAX_PLYR] = {          /* Computer plane territory         */
        0, 1155, 0,    2089
};
int     rcompter[MAX_PLYR] = {          /* Computer plane territory         */
        0, 2088, 1154, 10000
};

OBJECTS *objsmax        =       0;      /* Maximum object allocated         */
int     endsts[MAX_PLYR];               /* End of game status and move count*/
int     endcount;
int     player;                         /* Pointer to player's object       */
int     currobx;                        /* Current object index             */
BOOL    plyrplane;                      /* Current object is player flag    */
BOOL    compplane;                      /* Current object is a comp plane   */
OLDWDISP wdisp[MAX_OBJS];               /* World display status             */
BOOL    goingsun;                       /* Going to the sun flag            */
BOOL    forcdisp;                       /* Force display of ground          */
char    *histin, *histout;              /* History input and output files   */
unsigned explseed;                      /* random seed for explosion        */

int     keydelay = -1;                  /* Number of displays per keystroke */
int     dispcnt;                        /* Displays to delay keyboard       */
int     endstat;                        /* End of game status for curr. move*/
int     maxcrash;                       /* Maximum number of crashes        */
int     shothole;                       /* Number of shot holes to display  */
int     splatbird;                      /* Number of slatted bird symbols   */
int     splatox;                        /* Display splatted ox              */
int     oxsplatted;                     /* An ox has been splatted          */

int     sintab[ANGLES] = {              /* sine table of pi/8 increments    */
        0,      98,     181,    237,    /*   multiplied by 256              */
        256,    237,    181,    98,
        0,      -98,    -181,   -237,
        -256,   -237,   -181,   -98
};

jmp_buf envrestart;                     /* Restart environment for restart  */
                                        /*  long jump.                      */

extern  int      swkeyint();            /* Keyboard interrupt server        */

#ifdef  IBMPC                           /* System type                      */
extern  int      _systype;
#endif
#ifdef  ATARI
        int     _systype;
#endif


main( argc, argv )
int     argc;
char    *argv[];
{
        int nexttic;
        
        nobjects = (OBJECTS *)malloc( 100 * sizeof( OBJECTS ) );

        swinit( argc, argv );
        setjmp( envrestart );

        nexttic = Timer_GetMS();
        
        FOREVER {
		
                /*----- DLC 96/12/27 ------
                while ( movetick < 2  );
                movetick = 0;
                -------------------------*/
                //while ( movetick < movemax );

		// sdh: in the original, movetick was incremented
		// automagically by a timed interrupt. we dont
		// have interrupts so we have to pause between tics
		
                nexttic += 1000/FPS;
                do {
			swsndupdate();
		} while(Timer_GetMS() < nexttic);
              
                intsoff();
                movetick -= movemax;
                intson();

		// swmove and swdisp should be made to run
		// asyncronously probably

		swmove();
//                swgetjoy();
                swdisp();
                swgetjoy();
                swcollsn();
//                swgetjoy();
                intsoff();
                if ( printflg ) {
                        printflg = FALSE;
//                        _intreset( koveride );
                        intson();
                        swprint();
                        intsoff();
//                        koveride = _intsetup( KEYINT, swkeyint,
//                                              csseg(), dsseg() );
                }
                intson();
                swsound();
        }
}

