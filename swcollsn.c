/*
        swcollsn -      SW collision resolution

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-02        Development
                        84-06-12        PCjr Speed-up
                        84-10-31        Atari
                        87-03-09        Microsoft compiler.
                        87-03-11        No explosion on bird-plane collision
                        87-03-12        Wounded airplanes.
                        87-03-12        More than 1 bullet to kill target.
                        87-03-13        Splatted bird symbol.
                        87-03-31        Missiles.
                        87-04-05        Missile and starburst support
*/
#include        "sw.h"


extern  OBJECTS topobj, botobj;
extern  OBJECTS oobjects[];             /* Original plane object description*/
extern  OBJECTS *nobjects;              /* Objects list.                    */
extern  GRNDTYPE ground[];              /*  Ground height by pixel          */
extern  GRNDTYPE orground[];            /* Original ground height by pixel  */
extern  int     playmode;
extern  int     player;
extern  int     shothole;               /* Number of window shots to dislay */
extern  int     splatbird;              /* Number of splatted birds         */
extern  int     splatox;                /* Display a splatted OX            */
extern  int     numtarg[];              /*  Number of active targets        */
extern  MULTIO  *multbuff;
extern  int     gamenum;                /* Current game number              */
extern  int     endsts[];               /* End of game status and move count*/
extern  int     forcdisp;               /* Force display of ground          */
extern  int     gmaxspeed,gminspeed;    /* Speed range based on game number */
extern  int     counttick, countmove;   /* Performance counters            */

static  OBJECTS *killed[MAX_OBJS << 1],
                *killer[MAX_OBJS << 1];
static  int     killptr;

static  int     collsdx[MAX_PLYR];
static  int     collsdy[MAX_PLYR];
static  OBJECTS *collsno[MAX_PLYR];
static  int     collptr;
static  int     collxadj, collyadj;



swcollsn()
{
register OBJECTS *ob, *obp, **obkd, **obkr;
register int     xmax, ymin, ymax, otype, i;
int              prevx1, prevx2;

        collptr = killptr = 0;
        collxadj = 2;
        collyadj = 1;
        if ( countmove & 1 ) {
                collxadj = -collxadj;
                collyadj = -collyadj;
        }
        setadisp();
        prevx1 = topobj.ob_x;
        for ( ob = topobj.ob_xnext; ob != &botobj; ob = ob->ob_xnext ) {
                prevx2 = prevx1 = ob->ob_x;

                xmax = ob->ob_x + ob->ob_symwdt - 1;
                ymin = ( ymax = ob->ob_y ) - ob->ob_symhgt + 1;

                for ( obp = ob->ob_xnext;
                        ( obp != &botobj ) && ( obp->ob_x <= xmax );
                        obp = obp->ob_xnext ) {
                        prevx2 = obp->ob_x;

                        if ( ( obp->ob_y >= ymin )
                          && ( ( obp->ob_y - obp->ob_symhgt + 1 ) <= ymax ) )
                                colltest( ob, obp );
                }

                if ( ( ( ( otype = ob->ob_type ) == PLANE )
                            && ( ob->ob_state != FINISHED )
                            && ( ob->ob_state != WAITING )
                            && ( ob->ob_y < ( ground[ob->ob_x + 8] + 24 ) ) )
                          || ( ( ( otype == BOMB ) || ( otype == MISSILE ) )
                            && ( ob->ob_y < ( ground[ob->ob_x + 4] + 12 ) ) ) )
                        tstcrash( ob );
        }

        obkd = killed;
        obkr = killer;
        for ( i = 0; i < killptr; ++i, ++obkd, ++obkr )
               swkill( *obkd, *obkr );

        obkd = collsno;
        for ( i = 0; i < collptr; ++i, ++obkd ) {
                ( ob = *obkd )->ob_dx = collsdx[i];
                ob->ob_dy = collsdy[i];
        }
}




colltest( ob1, ob2 )
OBJECTS *ob1, *ob2;
{
register OBJECTS *obt, *ob, *obp;
register int     otype, ttype;

        ob = ob1;
        obp = ob2;
        otype = ob->ob_type;
        ttype = obp->ob_type;
        if ( ( ( otype == PLANE ) && ( ob->ob_state >= FINISHED ) )
                || ( ( ttype == PLANE ) && ( obp->ob_state >= FINISHED ) )
                || ( ( otype == EXPLOSION ) && ( ttype == EXPLOSION ) ) )
                return;

        if ( ob->ob_y < obp->ob_y ) {
                obt = ob;
                ob = obp;
                obp = obt;
        }

        swputsym( 15, 15, ob );
        if ( swputcol( obp->ob_x - ob->ob_x + 15,
                   obp->ob_y - ob->ob_y + 15,
                   obp ) )
                if ( killptr < ( ( MAX_OBJS << 1 ) - 1 ) ) {
                        killed[killptr] = ob;
                        killer[killptr++] = obp;
                        killed[killptr] = obp;
                        killer[killptr++] = ob;
                }
        swclrcol();
}



tstcrash( obp )
OBJECTS *obp;
{
register OBJECTS *ob;
register int     x, xmax, y;
register BOOL    hit = FALSE;

        ob = obp;
        swputsym( 15, 15, ob );

        xmax = ob->ob_x + ob->ob_symwdt - 1;
        for ( x = ob->ob_x; x <= xmax; ++x ) {
                if ( ( y = (int) ground[x] - ob->ob_y + 15 ) > 15 ) {
                        hit = TRUE;
                        break;
                }
                if ( y < 0 )
                        continue;
                if ( hit = swpntcol( x - ob->ob_x + 15, y, 0x80 ) )
                        break;
        }
        swclrcol();

        if ( ( hit ) && ( killptr < ( MAX_OBJS << 1 ) ) ){
                killed[killptr] = ob;
                killer[killptr++] = NULL;
        }
}

// sdh -- renamed this to swkill to remove possible conflicts with
// the unix kill() function

swkill( ob1, ob2 )
OBJECTS *ob1, *ob2;
{
register OBJECTS *ob, *obt;
register int     state, ttype, i;

        ob = ob1;
        obt = ob2;
        ttype = obt ? obt->ob_type : GROUND;
        if ( ( ( ttype == BIRD ) || ( ttype == FLOCK ) )
                && ( ob->ob_type != PLANE ) )
                return;

        switch ( ob->ob_type ) {

                case BOMB:
                case MISSILE:
                        initexpl( ob, 0 );
                        ob->ob_life = -1;
                        if ( !obt )
                                crater( ob );
                        stopsound( ob );
                        return;

                case SHOT:
                        ob->ob_life = 1;
                        return;

                case STARBURST:
                        if ( ( ttype == MISSILE ) || ( ttype == BOMB ) || !obt )
                                ob->ob_life = 1;
                        return;

                case EXPLOSION:
                        if ( !obt ) {
                                ob->ob_life = 1;
                                stopsound( ob );
                        }
                        return;

                case TARGET:
                        if ( ob->ob_state != STANDING )
                                return;
                        if ( ( ttype == EXPLOSION ) || ( ttype == STARBURST ) )
                                return;

                        if ( ( ttype == SHOT )
                                && ( ( ob->ob_hitcount += TARGHITCOUNT )
                                   <= ( TARGHITCOUNT * ( gamenum + 1 ) ) ) )
                                   return;

                        ob->ob_state = FINISHED;
                        initexpl( ob, 0 );

                        setvdisp();
                        dispwobj( ob );
                        setadisp();

                        scoretarg( ob, ( ob->ob_orient == 2 ) ? 200 : 100 );
                        if ( !--numtarg[ob->ob_clr - 1] )
                                endgame( ob->ob_clr );
                        return;

                case PLANE:
                        if ( ( ( state = ob->ob_state ) == CRASHED )
                                || ( state == GHOSTCRASHED ) )
                                return;

                        if ( endsts[ob->ob_index] == WINNER )
                                return;

                        if ( ( ttype == STARBURST )
                                || ( ( ttype == BIRD ) && ob->ob_athome ) )
                                return;

                        if ( !obt ) {
                                if ( state == FALLING ) {
                                        stopsound( ob );
                                        initexpl( ob, 1 );
                                        crater( ob );
                                }  else
                                        if ( state < FINISHED ) {
                                                scorepln( ob );
                                                initexpl( ob, 1 );
                                                crater( ob );
                                        }

                                crashpln( ob );
                                return;
                        }

                        if ( state >= FINISHED )
                                return;

                        if ( state == FALLING ) {
                                if ( ob->ob_index == player )
                                        if ( ttype == SHOT )
                                                ++shothole;
                                        else if ( ( ttype == BIRD )
                                                || ( ttype == FLOCK ) )
                                                ++splatbird;
                                return;
                        }

                        if ( ( ttype == SHOT ) || ( ttype == BIRD )
                                || ( ttype == OX ) || ( ttype == FLOCK ) ) {
                                if ( ob->ob_index == player )
                                        if ( ttype == SHOT )
                                                ++shothole;
                                        else if ( ttype == OX )
                                                ++splatox;
                                        else
                                                ++splatbird;
                                if ( state == FLYING ) {
                                        ob->ob_state = WOUNDED;
                                        return;
                                }
                                if ( state == STALLED ) {
                                        ob->ob_state = WOUNDSTALL;
                                        return;
                                }
                        } else {
                                initexpl( ob, 1 );
                                if ( ttype == PLANE ) {
                                        collsdx[collptr]
                                            = ( ( ob->ob_dx + obt->ob_dx ) >> 1)
                                            + ( collxadj = -collxadj );
                                        collsdy[collptr]
                                            = ( ( ob->ob_dy + obt->ob_dy ) >> 1)
                                            + ( collyadj = -collyadj );
                                        collsno[collptr++] = ob;
                                }
                        }

                        hitpln( ob );
                        scorepln( ob );
                        return;

                case BIRD:
                        ob->ob_life = scorepenalty( ttype, obt, 25 ) ? -1 : -2;
                        return;

                case FLOCK:
                        if ( ( ttype != FLOCK ) && ( ttype != BIRD )
                                && ( ob->ob_state == FLYING ) ) {
                                for ( i = 0; i < 8; ++i )
                                        initbird( ob, i );
                                ob->ob_life = -1;
                                ob->ob_state = FINISHED;
                                }
                        return;

                case OX:
                        if ( ob->ob_state != STANDING )
                                return;
                        if ( ( ttype == EXPLOSION ) || ( ttype == STARBURST ) )
                                return;
                        scorepenalty( ttype, obt, 200 );
                        ob->ob_state = FINISHED;
                        return;
        }
}



static  scorepenalty( ttype, ob, score )
int     ttype;
OBJECTS *ob;
int     score;
{
register OBJECTS *obt;

        obt = ob;
        if ( ( ttype == SHOT ) || ( ttype == BOMB ) || ( ttype ==  MISSILE )
                || ( ( ttype == PLANE )
                    && ( ( obt->ob_state == FLYING )
                        || ( obt->ob_state == WOUNDED )
                        || ( ( obt->ob_state == FALLING )
                                && ( obt->ob_hitcount == FALLCOUNT ) ) )
                    && ( !obt->ob_athome ) ) ) {
                scoretarg( obt, score );
                return( TRUE );
        }
        return( FALSE );
}




static  scoretarg( obp, score )
OBJECTS *obp;
int     score;
{
register OBJECTS *ob;

        ob = obp;
        if ( ( ( playmode != MULTIPLE ) && ( playmode != ASYNCH ) )
                || ( multbuff->mu_maxplyr == 1 ) ) {
                if ( ob->ob_clr == 1 )
                        nobjects[0].ob_score -= score;
                else
                        nobjects[0].ob_score += score;
                dispscore( &nobjects[0] );
        } else {
                nobjects[2 - ob->ob_clr].ob_score += score;
                dispscore( &nobjects[2 - ob->ob_clr] );
        }
}





scorepln( ob )
OBJECTS *ob;
{

        scoretarg( ob, 50 );
}




dispscore( obp )
OBJECTS *obp;
{
register OBJECTS *ob;

        swposcur( ( ( ob = obp )->ob_clr - 1 ) * 7 + 2, 24 );
        swcolour( ob->ob_clr );
        dispd( ob->ob_score, 6 );
}




dispd( n, size )
int     n, size;
{
register int   i     = 0;
register int   d, t;
register BOOL  first = TRUE;

        if ( n < 0 ) {
                n = -n;
                swputc( '-' );
                ++i;
        }
        for ( t = 10000; t > 1; n %= t, t /= 10 )
                if ( ( d = n / t ) || ( !first ) ){
                        first = FALSE;
                        swputc( d + '0' );
                        ++i;
                }
        swputc( n + '0' );
        ++i;
        while( ++i <= size )
                swputc( ' ' );
}






static crtdepth[8] = { 1, 2, 2, 3, 3, 2, 2, 1 };

static crater( ob )
OBJECTS *ob;
{
register int    i, x, y, ymin, ymax;
int             xmin, xmax;

        xmin = ob->ob_x + ( ob->ob_symwdt - 8 ) / 2;
        xmax = xmin + 7;

        for ( x = xmin, i = 0; x <= xmax; ++x, ++i ) {
                ymax = ground[x];
                if ( ( y = orground[x] - 20 ) < 20 )
                        y = 20;
                if ( ( ymin = ymax - crtdepth[i] + 1 ) <= y )
                        ymin = y + 1;
                ground[x] = ymin - 1;
        }
        forcdisp = TRUE;
}


// sdh: wtf?

equal( x, y )
int     ( *x )(), ( *y )();
{
        return ( x == y );
}

