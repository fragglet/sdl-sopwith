/*
        swauto   -      SW control of computer player

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-03-05        Development
                        84-06-12        PCjr Speed-up
                        84-10-31        Atari
                        87-03-09        Microsoft compiler.
                        87-03-12        Wounded airplanes.
                        87-03-12        Smarter shooting.
                        87-03-31        Missiles.
                        87-04-05        Missile and starburst support
                        87-04-06        Computer plane avoiding oxen.
                        87-04-09        Don't attack until closer.
*/
#include        "sw.h"

extern  int     sintab[];               /* sine table based on angles    */
extern  GRNDTYPE ground[];              /* Ground height by pixel        */
extern  GRNDTYPE orground[];            /* Minimum ground height in craters */
extern  OBJECTS *nobjects;              /* Objects list.                    */
extern  OBJECTS oobjects[];             /* Original plane object description*/
extern  OBJECTS *compnear[];            /* Array of planes near computers   */
extern  OBJECTS *targets[];             /* Target status array              */
extern  int     lcompter[];             /* Computer plane territory       */
extern  int     rcompter[];             /* Computer plane territory       */
extern  OBJECTS topobj, botobj;
extern  int     dispcomp(), dispplyr();
extern  int     keydelay;               /*  Number of displays per keystr   */
extern  int     playmode;               /*  Play mode                       */
extern  BOOL    goingsun;               /* Heading for the sun flag         */
extern  int     endcount;
extern  int     gmaxspeed,gminspeed;    /* Speed range based on game number */
extern  BOOL    compplane;              /* Moving computer plane flag       */
extern  BOOL    plyrplane;              /* Moving player plane flag         */
extern  int     currobx;                /* Current object index             */
extern  int     counttick, countmove;   /* Performance counters            */


static  BOOL    correction;             /*  Course correction flag        */
static  BOOL    goinghome;              /*  Going home flag               */
static  OBJECTS obs;                    /*  Saved computer object         */
static  int     courseadj;              /*  Course adjustment             */




swauto( ob )
OBJECTS *ob;
{
register OBJECTS *obt;

        goinghome = FALSE;

        if ( obt = compnear[currobx] )
                attack( ob, obt );
        else
                if ( !ob->ob_athome )
                       cruise( ob );

        compnear[currobx] = NULL;
}





attack( obp, obt )
OBJECTS *obp, *obt;
{
register OBJECTS *ob;

        courseadj = ( ( countmove & 0x001F ) < 16 ) << 4;
        ob = obt;
        if ( ob->ob_speed )
                aim( obp,
                     ob->ob_x - ( ( CLOSE * COS( ob->ob_angle ) ) >> 8 ),
                     ob->ob_y - ( ( CLOSE * SIN( ob->ob_angle ) ) >> 8 ),
                     ob, NO );
        else
                aim( obp, ob->ob_x, ob->ob_y + 4, ob, NO );
}




static  cruise( ob )
OBJECTS *ob;
{
register int     orgx;

        courseadj = ( ( countmove & 0x001F ) < 16 ) << 4;
        orgx = oobjects[currobx].ob_x;
        aim( ob, ( ( orgx < ( MAX_X / 3 ) )
                ? ( MAX_X / 3 )
                : ( ( orgx > ( 2 * MAX_X / 3 ) )
                ? ( 2 * MAX_X / 3 )
                : orgx ) ) + courseadj,
                MAX_Y - 50 - ( courseadj >> 1 ),
                NULL, NO );
}




gohome( obpt )
OBJECTS *obpt;
{
register OBJECTS *ob, *obp;

        if ( ( obp = obpt )->ob_athome )
                return( 0 );

        ob = &oobjects[currobx];

        courseadj = ( ( countmove & 0x001F ) < 16 ) << 4;
        if ( ( abs( obp->ob_x - ob->ob_x ) < HOME )
                && ( abs( obp->ob_y - ob->ob_y ) < HOME ) ) {
                if ( plyrplane ) {
                        initplyr( obp );
                        initdisp( YES );
                } else
                        if ( compplane )
                                initcomp( obp );
                        else
                                initpln( obp );
                return( 0 );
        }
        goinghome = TRUE;
        return( aim( obp, ob->ob_x, ob->ob_y, NULL, NO ) );
}





shoot( obt )
OBJECTS *obt;
{
static          OBJECTS obsp, obtsp;
int             obx, oby, obtx, obty;
int             nspeed, nangle;
int             rprev;
register int    r, i;


        movmem( &obs, &obsp, sizeof( OBJECTS ) );
        movmem( obt, &obtsp, sizeof( OBJECTS ) );
        nspeed = obsp.ob_speed + BULSPEED;
        setdxdy( &obsp, nspeed * COS( obsp.ob_angle ),
                        nspeed * SIN( obsp.ob_angle ) );
        obsp.ob_x += SYM_WDTH / 2;
        obsp.ob_y -= SYM_HGHT / 2;

        nangle = obtsp.ob_angle;
        nspeed = obtsp.ob_speed;
        rprev = NEAR;
        for ( i = 0; i < BULLIFE; ++i ) {
                movexy( &obsp, &obx, &oby );
                if ( ( ( obtsp.ob_state == FLYING )
                        || ( obtsp.ob_state == WOUNDED ) )
                        && ( r = obtsp.ob_flaps ) ) {
                        if ( obtsp.ob_orient )
                                nangle -= r;
                        else
                                nangle += r;
                        nangle = ( nangle + ANGLES ) % ANGLES;
                        setdxdy( &obtsp, nspeed * COS( nangle ),
                                         nspeed * SIN( nangle ) );
                }
                movexy( &obtsp, &obtx, &obty );
                r = range( obx, oby, obtx, obty );
                if ( ( r < 0 ) || ( r > rprev ) )
                        return( 0 );
                if ( ( obx >= obtx )
                        && ( obx <= ( obtx + SYM_WDTH - 1 ) )
                        && ( oby <= obty )
                        && ( oby >= ( obty - SYM_HGHT + 1 ) ) )
                        return( 1 + ( i > ( BULLIFE / 3 ) ) );

        }
        return( 0 );
}




abs( x )
int     x;
{
        return( ( x < 0 ) ? -x : x );
}







aim( obo, ax, ay, obt, longway )
OBJECTS *obo, *obt;
int     ax, ay;
BOOL    longway;
{
register OBJECTS *ob, *obp;
register int     r, rmin, i, n;
int     x, y, dx, dy, nx, ny;
int     nangle, nspeed;
static  int     cflaps[3] = { 0, -1, 1 };
static  int     crange[3],ccrash[3], calt[3];

        ob = obo;

        correction = FALSE;

        if ( ( ( ob->ob_state == STALLED ) || ( ob->ob_state == WOUNDSTALL ) )
                && ( ob->ob_angle != ( 3*ANGLES/4 ))){
                ob->ob_flaps = -1;
                ob->ob_accel = MAX_THROTTLE;
                return;
        }

        x = ob->ob_x;
        y = ob->ob_y;
        if ( abs( dx = x - ax ) > 160 ) {
                if ( ob->ob_dx && ( ( dx < 0 ) == ( ob->ob_dx < 0 ) ) ) {
                        if ( !ob->ob_hitcount )
                                ob->ob_hitcount = 1 + ( y > ( MAX_Y - 50 ) );
                        return( aim( ob, x, ( ob->ob_hitcount == 1 )
                                ? ( y + 25 ) : ( y - 25 ), NULL, YES ) );
                }
                ob->ob_hitcount = 0;
                return( aim( ob, x + ( ( dx < 0 ) ? 150 : -150 ),
                             ( ( ( y += 100 ) > ( MAX_Y - 50 - courseadj ) )
                             ? ( MAX_Y - 50 - courseadj ) : y ), NULL, YES ) );
        } else
                if ( !longway )
                        ob->ob_hitcount = 0;

        if ( ob->ob_speed )
                if ( correction = ( dy = y - ay ) && ( abs( dy ) < 6 ) )
                        ob->ob_y = ( dy < 0 ) ? ++y : --y;
                else
                        if ( correction = dx && ( abs( dx ) < 6 ) )
                                ob->ob_x = ( dx < 0 ) ? ++x : --x;

        movmem( ob, &obs, sizeof( OBJECTS ) );
        if ( ( ( nspeed = obs.ob_speed + 1 ) > gmaxspeed )
                && ( obs.ob_type == PLANE ) )
                nspeed = gmaxspeed;
        else
                if ( nspeed < gminspeed )
                        nspeed = gminspeed;

        cleartargs();
        for ( i = 0; i < 3; ++i ) {
                nangle = ( obs.ob_angle
                         + ( obs.ob_orient ? -( cflaps[i] ) : cflaps[i] )
                         + ANGLES ) % ANGLES;
                setdxdy( &obs, nspeed * COS( nangle ), nspeed * SIN( nangle ) );
                movexy( &obs, &nx, &ny );
                crange[i] = range( nx, ny, ax, ay );
                calt[i] = ny - orground[nx + 8];
                ccrash[i] = tstcrash( ob, nx, ny, calt[i] );
                movmem( ob, &obs, sizeof( OBJECTS ) );
        }


        if ( obt && ( i = shoot( obt ) ) )
                if ( ob->ob_missiles && ( i == 2 ) )
                        ob->ob_mfiring = obt->ob_athome ? ob : obt;
                else
                        ob->ob_firing = obt;

        rmin = 32767;
        for ( i = 0; i < 3; ++i )
                if ( ( ( r = crange[i] ) >= 0 )
                        && ( r < rmin )
                        && !ccrash[i] ) {
                        rmin = r;
                        n = i;
                }
        if ( rmin == 32767 ) {
                rmin = -32767;
                for ( i = 0; i < 3; ++i )
                        if ( ( ( r = crange[i] ) < 0 )
                                && ( r > rmin )
                                && !ccrash[i] ) {
                                rmin = r;
                                n = i;
                        }
        }

        if ( ob->ob_speed < gminspeed )
                ob->ob_accel = MAX_THROTTLE;

        if ( rmin != -32767 ) {
                if ( ob->ob_accel < MAX_THROTTLE )
                        ++ob->ob_accel;
        } else {
                if ( ob->ob_accel )
                        --ob->ob_accel;

                n = 0;
                if ( calt[1] > ( dy = calt[0] ) ) {
                        dy = calt[1];
                        n = 1;
                }
                if ( calt[2] > dy )
                        n = 2;
        }

        ob->ob_flaps = cflaps[n];
        if ( ( ob->ob_type == PLANE ) && !ob->ob_flaps )
                if ( ob->ob_speed )
                        ob->ob_orient = ( ob->ob_dx < 0 );

}



static  int     tl, tr;



static  cleartargs()
{
        tl = -2;
}



static  testtargs( x, y )
int     x, y;
{
register int    i, xl, xr;

        xl = x - 32 - gmaxspeed;
        xr = x + 32 + gmaxspeed;

        tl = -1;
        tr = 0;
        for ( i = 0; i < ( MAX_TARG + MAX_OXEN ); ++i )
                if ( targets[i] && ( targets[i]->ob_x >= xl ) ) {
                        tl = i;
                        break;
                }

        if ( tl == -1 )
                return;

        for ( ; ( i < ( MAX_TARG + MAX_OXEN ) ) && targets[i]
                && ( targets[i]->ob_x < xr ); ++i );
        tr = i - 1;
}




static  tstcrash( obp, x, y, alt )
int     x, y, alt;
OBJECTS *obp;
{
register OBJECTS *ob;
register int     i, xl, xr, xt, yt;

        if ( alt > 50 )
                return( FALSE );

        if ( alt < 22 )
                return( TRUE );

        ob = obp;
        if ( tl == -2 )
                testtargs( ob->ob_x, ob->ob_y );

        xl = x - 32;
        xr = x + 32;

        for ( i = tl; i <= tr; ++i ) {
                if ( ( xt = ( ob = targets[i] )->ob_x ) < xl )
                        continue;
                if ( xt > xr )
                        return( FALSE );
                yt = ob->ob_y + ( ob->ob_state == STANDING ? 16 : 8 );
                if ( y <= yt )
                        return( TRUE );
        }
        return( FALSE );
}





range( x, y, ax, ay )
int     x, y, ax, ay;
{
register int     dx, dy;
register int     t;

        dy = abs( y - ay );
        dy += dy >> 1;
        if ( ( ( dx = abs( x - ax ) ) < 125 ) && ( dy < 125 ) )
                return ( dx * dx + dy * dy );

        if ( dx < dy ) {
                t = dx;
                dx = dy;
                dy = t;
        }

        return( - ( ( ( 7 * dx ) + ( dy << 2 ) ) >> 3 ) );
}
