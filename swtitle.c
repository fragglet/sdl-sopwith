/*
        swtitle  -      SW perform animation on the title screen

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-02        Development
                        87-03-10        Microsoft compiler.
                        87-03-11        Title reformatting.
                        87-04-01        Version 7.F15
                        96-12-27        New network version.
                        99-01-24        1999 copyright.
                        2000-10-29      Copyright update.
*/
#include        "sw.h"
#include        "cgavideo.h"

// sdh -- use network edition title screen
//#define NET_TITLE

// sdh: move these into headers

extern  int             savemode;               /* Saved PC video mode    */
extern  BOOL            hires;                  /* High res debug flag    */
extern  BOOL            titleflg;               /* Title flag             */
extern  int             tickmode;               /* Tick action to be done */
extern  int             counttick, countmove;   /* Performance counters   */
extern  int             movetick;               /* Move synchronization   */
extern  int     displx, disprx;         /* Display left and right bounds  */
extern  BOOL    dispinit;               /* Initalized display flag.       */
extern  GRNDTYPE ground[];              /* Ground height by pixel         */




swtitln()
{
	OBJECTS         ob;
	extern   char   swplnsym[][ANGLES][SYMBYTES];
	extern   char   swtrgsym[][TARGBYTES];
	extern   char   swoxsym[][OXBYTES];
	extern   char   swhitsym[][SYMBYTES];
	extern   char   swwinsym[][WINBYTES];
	register int     i, h;
 
        savemode = get_type();
        set_type ( ( hires ) ? 6 : 4 );

        if ( titleflg )
                return;

        tickmode = 1;

        sound( S_TITLE, 0, NULL );
        swsound();

/*---------------- Original BMB Version---------------*/
#ifndef NET_TITLE
	
        swcolour( 3 );
        swposcur( 13, 6 );
        swputs( "S O P W I T H" );

        swcolour( 1 );
        swposcur( 12, 8 );
        swputs( "(Version 7.F15)" );

        swcolour( 3 );
        swposcur( 5, 11 );
        swputs( "(c) Copyright 1984, 1985, 1987" );

        swcolour( 1 );
        swposcur( 6, 12 );
        swputs( "BMB " );
        swcolour( 3 );
        swputs( "Compuscience Canada Ltd." );
#else
/*------------------ Original BMB Version---------------*/

/*---------------- New Network Version ---------------*/

        swcolour( 3 );
        swposcur( 13, 4 );
        swputs( "S O P W I T H" );

        swcolour( 1 );
        swposcur( 9, 6 );
        swputs( "(Distribution Version)" );

        swcolour( 3 );
        swposcur( 5, 9 );
        swputs( "(c) Copyright 1984, 1985, 1987" );

        swcolour( 1 );
        swposcur( 6, 10 );
        swputs( "BMB " );
        swcolour( 3 );
        swputs( "Compuscience Canada Ltd." );

        swcolour( 3 );
        swposcur( 1, 12 );
        swputs( "(c) Copyright 1984-2000 David L. Clark" );

/*---------------- New Network Version-----------------*/

        setvdisp();

        displx = 700;
        dispinit = TRUE;
        swground();

        ob.ob_type = PLANE;
        ob.ob_symhgt = ob.ob_symwdt = 16;
        ob.ob_clr = 1;
        ob.ob_newsym = swplnsym[0][0];
        swputsym( 260, 180, &ob );

        ob.ob_newsym = swwinsym[3];
        swputsym( 50, 180, &ob );

        ob.ob_type = OX;
        ob.ob_newsym = swoxsym[0];
        swputsym( 100, ground[800] + 16, &ob );

        ob.ob_type = TARGET;
        ob.ob_clr = 2;
        ob.ob_newsym = swtrgsym[3];
        swputsym( 234, ground[934] + 16, &ob );

        ob.ob_type = PLANE;
        ob.ob_newsym = swhitsym[0];
        swputsym( 20, 160, &ob );

        ob.ob_type = SMOKE;
        ob.ob_symhgt = ob.ob_symwdt = 1;
        ob.ob_newsym = (char *)0x82;
        h = 150;
        for ( i = 9; i; --i )
                swputsym( 30, h += 5, &ob );

#endif /* #ifndef NET_TITLE */

	// sdh: need update to show the screen
	
	CGA_Update();

}

swtitlf()
{

        if ( titleflg )
                return;

        sound( 0, 0, NULL );
        swsound();
        tickmode = 0;
}

// sdh -- this function was called on a timed interrupt
// in the original to keep the game moving and update
// sounds. it is unused now

swtickc()
{

        ++counttick;
        /*--- DLC 96/12/27
        ++movetick;
        ----------------*/
        movetick+=10;

        soundadj();
}

