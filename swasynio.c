/*
        swasynio -      SW asynchrounous communications I/O

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        85-04-03        Development
                        87-03-09        Microsoft compiler.
                        87-03-12        Allow asynch loopback for debugging.
                        96-12-26        Remove IMAGINET network card
                                          card address in "multaddr".
*/
#include        "sw.h"




extern  int     player;                 /* Pointer to player's object       */
extern  GAMES   swgames[], *currgame;   /* Game parameters and current game */
extern  int     counttick, countmove;   /* Performance counters             */
extern  MULTIO  *multbuff;              /* Communications buffer            */
extern  OBJECTS oobjects[];             /* Original plane object description*/
extern  int     dispmult(),             /*  Display and move functions      */
                movemult();
extern  unsigned explseed;              /* explosion seed                   */
extern  int     multaddr;               /* port address override            */

static  int     lastkey = 0;            /*  Always behind one character     */


static  synchronize()
{
register int     syncount;
register int     c;

        while ( commin() >= 0 );
        syncount = 0;
        FOREVER {
                settimeout( 2 );
                while ( !timeout() );
                commout( K_ASYNACK );

                if ( ctlbreak() )
                        swend( NULL, NO );

                if ( ( c = commin() ) < 0 )
                        continue;

                if ( c == K_ASYNACK ) {
                        if ( ++syncount == 10 )
                                break;
                        continue;
                }
                syncount = 0;
        }
        commout( 0 );

        FOREVER {
                if ( ( c = asynin() ) == -1 )
                        swend( "Time out on sychronization", NO );
                if ( c != K_ASYNACK )
                        break;
        }
}





static  tickwait;



static  settimeout( tick )
int     tick;
{
        intsoff();
        tickwait = tick;
        counttick = 0;
        intson();
}




static  timeout()
{
        return( tickwait < counttick );
}





static  asynin()
{
register int     c;

        settimeout( 180 );
        FOREVER {
                if ( ( c = commin() ) >= 0 )
                        return( c );
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( timeout() )
                        return( -1 );
        }
}





asynget( ob )
OBJECTS *ob;
{
register int     key;
register int     c;


        if ( ob->ob_index == player ) {
                key = lastkey;
                lastkey = 0;
        } else {
                if ( ( ( key = asynin() ) < 0 )
                        || ( ( c = asynin() ) < 0 ) )
                        swend( "Timeout during play", NO );
                key = key | ( c << 8 );
        }
        return( histmult( ob->ob_index, key ) );
}




asynput()
{
static  BOOL    first = TRUE;


        if  ( first )
                first = FALSE;
        else
                lastkey = swgetc();
        swflush();

        commout( lastkey & 0x00FF );
        commout( lastkey >> 8 );
}





char    *asynclos( update )
BOOL    update;
{
        commterm();
        return( NULL );
}




init1asy()
{
unsigned          seed;
register int      c1, c2;

        comminit();
        clrprmpt();
        puts( "        Waiting for other player" );
        synchronize();

        commout( explseed & 0x00FF );
        commout( explseed >> 8 );
        if ( ( ( c1 = asynin() ) < 0 )
                || ( ( c2 = asynin() ) < 0 ) )
                swend( "Timeout during player assignment", NO );
        seed = c1 | ( c2 << 8 );
        if ( player = ( seed > explseed ) )
                explseed = seed;

        currgame = &swgames[0];
        multbuff->mu_numplyr = multbuff->mu_maxplyr = 2;
}



init2asy()
{
register OBJECTS *ob;
OBJECTS          *initpln();

        if ( !player )
                initplyr( NULL );

        ob = initpln( NULL );
        ob->ob_drawf = dispmult;
        ob->ob_movef = movemult;
        ob->ob_clr = ob->ob_index % 2 + 1;
        ob->ob_owner = ob;
        ob->ob_state = FLYING;
        movmem( ob, &oobjects[ob->ob_index], sizeof( OBJECTS ) );

        if ( player )
                initplyr( NULL );

        commout( 0 );
        commout( 0 );
}
