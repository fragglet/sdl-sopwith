/*
        swend    -      SW end of game

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-02        Development
                        87-03-09        Microsoft compiler.
                        87-03-12        Wounded airplanes.
*/
#include        "sw.h"




extern  int     playmode;               /* Mode of play ( SINGLE, MULTIPLE, */
                                        /*                or COMPUTER )     */
extern  int     savemode;               /* Saved PC video mode              */
extern  BOOL    hires;                  /* High res debug/mono flag         */
extern  OBJECTS *objtop;                /*  Start of object list.           */
extern  MULTIO  *multbuff;              /*  Communications buffer           */
extern  int     player;
extern  int     endsts[];               /* End of game status and move count*/
extern  int     endcount;
extern  BOOL    goingsun;               /* Heading for the sun flag         */
extern  OBJECTS *objsmax;               /* Maximum object allocated         */
extern  BOOL    repflag;                /* Report statictics flag           */
extern  BOOL    inplay;                 /*  Currently playing flag          */
extern  int     maxcrash;               /* Maximum number of crashes        */



swend( msg, update )
char    *msg;
BOOL    update;
{
register char   *closmsg = NULL;
char            *multclos(), *asynclos();


        set_type( savemode );
        hires = FALSE;

        sound( 0, 0, NULL );
        swsound();

        if ( repflag )
                swreport();

        if ( playmode == MULTIPLE )
                closmsg = multclos( update );
        else if ( playmode == ASYNCH )
                closmsg = asynclos();

        intsoff();
        _intterm();
        intson();
        histend();

        puts( "\r\n" );
        if ( closmsg ) {
                puts( closmsg );
                puts( "\r\n" );
        }
        if ( msg ) {
                puts( msg );
                puts( "\r\n" );
        }

        inplay = FALSE;
        swflush();
        if ( msg || closmsg )
                exit( YES );
        else
                exit( NO );
}





endgame( targclr )
int     targclr;
{
register int     winclr;
register OBJECTS *ob;

        if ( ( ( playmode != MULTIPLE ) && ( playmode != ASYNCH ) )
                || ( multbuff->mu_maxplyr == 1 ) )
                winclr = 1;
        else
                if ( ( objtop +1 )->ob_score == objtop->ob_score )
                        winclr = 3 - targclr;
                else
                        winclr = ( ( objtop + 1 )->ob_score > objtop->ob_score )
                                 + 1;

        ob = objtop;
        while ( ob->ob_type == PLANE ) {
                if ( !endsts[ob->ob_index] )
                        if ( ( ob->ob_clr == winclr )
                                  && ( ( ob->ob_crashcnt < ( MAXCRASH - 1 ) )
                               || ( ( ob->ob_crashcnt < MAXCRASH )
                                  && ( ( ob->ob_state == FLYING )
                                    || ( ob->ob_state == STALLED )
                                    || ( ob->ob_state == WOUNDED )
                                    || ( ob->ob_state == WOUNDSTALL )
                                ) ) ) )
                                winner( ob );
                        else
                                loser( ob );
                ob = ob->ob_next;
        }
}





winner( obp )
OBJECTS *obp;
{
register int     n;
register OBJECTS *ob;

        endsts[n = ( ob = obp )->ob_index] = WINNER;
        if ( n == player ) {
                endcount = 72;
                goingsun = TRUE;
                ob->ob_dx = ob->ob_dy = ob->ob_ldx = ob->ob_ldy = 0;
                ob->ob_state = FLYING;
                ob->ob_life = MAXFUEL;
                ob->ob_speed = MIN_SPEED;
        }
}




loser( ob )
OBJECTS *ob;
{
register int     n;

        endsts[n = ob->ob_index] = LOSER;
        if ( n == player ) {
                swcolour( 0x82 );
                swposcur( 16, 12 );
                puts( "THE END" );
                endcount = 20;
        }
}




swreport()
{
        puts( "\r\nEnd of game statictics\r\n\r\n" );
        puts( "Objects used: " );
        dispd( ( (int) objsmax - (int) objtop + 1 ) / sizeof( OBJECTS ) );
        puts( "\r\n" );
}
