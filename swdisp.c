/*
        swdispc  -      Display all players and objects

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-21        Development
                        84-06-12        PCjr Speed-up
                        87-03-09        Microsoft compiler.
                        87-03-12        Wounded airplanes.
                        87-03-13        Splatted bird symbol.
                        87-04-05        Missile and starburst support
*/
#include        "sw.h"



extern  OLDWDISP wdisp[];               /*  World display status            */
extern  OBJECTS *nobjects;              /*  Objects list.                   */
extern  OBJECTS *objtop;                /*  Start of object list.           */
extern  int     shothole;               /*  Number of shot holes to display */
extern  int     splatbird;              /*  Number of splatted birds        */
extern  int     splatox;                /* Display splatted ox              */
extern  int     oxsplatted;             /* An ox has been splatted          */
extern  char    swshtsym[];             /*  Shot hole symbol                */
extern  char    swsplsym[];             /*  Splatted bird symbol            */
extern  int     countmove;              /*  Move number                     */







dispplyr( ob )
OBJECTS *ob;
{
        if ( shothole )
                dispwindshot();
        if ( splatbird )
                dispsplatbird();

	// sdh: splatted ox is currently broken
//        if ( splatox )
//                dispoxsplat();
        plnsound( ob );
}






dispbomb( obp )
OBJECTS *obp;
{
register OBJECTS *ob;

        if ( ( ob = obp )->ob_dy <= 0 )
                sound( S_BOMB, -( ob->ob_y ), ob );
}





dispmiss( obp )
OBJECTS *obp;
{
}





dispburst( obp )
OBJECTS *obp;
{
}





dispexpl( obp )
OBJECTS *obp;
{
register OBJECTS *ob;

        if ( ( ob = obp )->ob_orient )
                sound( S_EXPLOSION, ob->ob_hitcount, ob );
}





dispcomp( ob )
OBJECTS *ob;
{
        plnsound( ob );
}




dispmult( ob )
OBJECTS *ob;
{
        plnsound( ob );
}




disptarg( ob )
OBJECTS *ob;
{
        if ( ob->ob_firing )
                sound( S_SHOT, 0, ob );
}




dispflck( ob )
OBJECTS *ob;
{
}




dispbird( ob )
OBJECTS *ob;
{
}





static  plnsound( obp )
OBJECTS *obp;
{
register OBJECTS *ob;

        ob = obp;
        if ( ob->ob_firing )
                sound( S_SHOT, 0, ob );
        else
                switch ( ob->ob_state ) {
                        case FALLING:
                                if ( ob->ob_dy >= 0 )
                                        sound( S_HIT, 0, ob );
                                else
                                        sound( S_FALLING, ob->ob_y, ob );
                                break;

                        case FLYING:
                                sound( S_PLANE, -( ob->ob_speed ), ob );
                                break;

                        case STALLED:
                        case WOUNDED:
                        case WOUNDSTALL:
                                sound( S_HIT, 0, ob );
                                break;

                        default:
                                break;
                }

}




dispwobj( obp )
OBJECTS *obp;
{
register OBJECTS  *ob;
register OLDWDISP *ow;
int               ox, oy, oldplot;

        ob = obp;
        ow = &wdisp[ob->ob_index];

        if ( ow->ow_xorplot )
                swpntsym( ow->ow_x, ow->ow_y, ow->ow_xorplot - 1 );

        if ( ob->ob_state >= FINISHED )
                ow->ow_xorplot = 0;
        else {
                oldplot = swpntcol( ow->ow_x = SCR_CENTR
                                    + ( ob->ob_x + ob->ob_symwdt / 2 )
                                    / WRLD_RSX,
                                    ow->ow_y
                                    = ( ob->ob_y - ob->ob_symhgt / 2 )
                                    / WRLD_RSY,
                                    ob->ob_owner->ob_clr );

                if ( ( oldplot == 0 ) || ( ( oldplot & 0x0003 ) == 3 ) ) {
                        ow->ow_xorplot = oldplot + 1;
                        return;
                }
                swpntsym( ow->ow_x, ow->ow_y, oldplot );
                ow->ow_xorplot = 0;
        }
}


static  unsigned long    seed = 74917777;

unsigned long   randsd()
{
        if ( !( seed = seed * countmove + 7491 ) )
                seed = 74917777;
}


extern char    swplnsym[ORIENTS][ANGLES][SYMBYTES];

dispwindshot()
{
	OBJECTS                  ob;
	
        ob.ob_type = DUMMYTYPE;
        ob.ob_symhgt = ob.ob_symwdt = 16;
        ob.ob_clr = 0;
        ob.ob_newsym = swshtsym;
        do {
                randsd();
                swputsym( (unsigned)( seed % ( SCR_WDTH - 16 ) ),
                          (unsigned)( seed % ( SCR_HGHT - 50 ) ) + 50,
                          &ob );
        } while ( --shothole );
}



dispsplatbird()
{
OBJECTS                  ob;

        ob.ob_type = DUMMYTYPE;
        ob.ob_symhgt = ob.ob_symwdt = 32;
        ob.ob_clr = 2;
        ob.ob_newsym = swsplsym;
        do {
                randsd();
                swputsym( (unsigned)( seed % ( SCR_WDTH - 32 ) ),
                          (unsigned)( seed % ( SCR_HGHT - 60 ) ) + 60,
                          &ob );
        } while ( --splatbird );
}




dispoxsplat()
{
        register OBJECTS *ob;
        register int     i;

        colorscreen(2);
        printf("oxsplat\n");
        
        swsetblk( 0,        SCR_SEGM,
                  ( ( SCR_HGHT - SCR_MNSH - 2 ) >> 1 ) * SCR_LINW, 0xAA );
        swsetblk( SCR_ROFF, SCR_SEGM,
                  ( ( SCR_HGHT - SCR_MNSH - 3 ) >> 1 ) * SCR_LINW, 0xAA );
        splatox = 0;
        oxsplatted = 1;

        ob = nobjects;
        for ( i = 0; i < MAX_OBJS; ++i, ob++ )
                ob->ob_drwflg = ob->ob_delflg = 0;
}

