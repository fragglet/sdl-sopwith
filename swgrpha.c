/*

        swgrph   -      SW screen graphics

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-21        Development
                        84-06-13        PCjr Speed-up
                        85-11-05        Atari
                        87-03-09        Microsoft compiler.
*/




#include        "sw.h"



extern  int     displx, disprx;         /* Display left and right bounds    */
extern  int     dispdx;                 /* Display shift                    */
extern  char    auxdisp[];              /* Auxiliary display area           */
extern  GRNDTYPE ground[];              /* Ground height by pixel           */
extern  BOOL    dispinit;               /* Initalized display flag.         */
extern  OBJECTS *objtop;                /* Top of object list               */
extern  OBJECTS *deltop;                /* Newly deallocated objects list   */
extern  int     forcdisp;               /* Force display of ground          */
extern  long    trap14();               /* BIOS trap                        */

static  char    *dispoff;               /* Current display offset           */
static  int     scrtype;                /* Screen type                      */
static  GRNDTYPE grndsave[SCR_WDTH];    /* Saved ground buffer for last     */
                                        /*   last display                   */
static  int     ( *dispg )();           /* display ground routine (mono,clr)*/
static  int     ( *drawpnt )();         /* draw point routine               */
static  int     ( *drawsym )();         /* draw symbol routine              */

static  int     palette[] = {   /* Colour palette                           */
        0x000,                  /*   0 = black    background                */
        0x037,                  /*   1 = blue     planes,targets,explosions */
        0x700,                  /*   2 = red      planes,targets,explosions */
        0x777,                  /*   3 = white    bullets                   */
        0x000,                  /*   4                                      */
        0x000,                  /*   5                                      */
        0x000,                  /*   6                                      */
        0x070,                  /*   7 = green    ground                    */
        0x000,                  /*   8                                      */
        0x433,                  /*   9 = tan      oxen, birds               */
        0x420,                  /*  10 = brown    oxen                      */
        0x320,                  /*  11 = brown    bottom of ground display  */
        0x000,                  /*  12                                      */
        0x000,                  /*  13                                      */
        0x000,                  /*  14                                      */
        0x000                   /*  15                                      */
};



static  char    spcbirds[BIRDSYMS][BRDBYTES*2];   /* Special bird symbol    */
                                                  /* colour video maps      */




/*---------------------------------------------------------------------------

        Main display loop.   Delete and display all visible objects.
        Delete any newly deleted objects

---------------------------------------------------------------------------*/



swdisp()
{
register OBJECTS *ob;

        setvdisp();
        for ( ob = objtop; ob; ob = ob->ob_next ) {
                if ( ( !( ob->ob_delflg && ob->ob_drwflg ) )
                        || ( ob->ob_symhgt == 1 )
                        || ( ob->ob_oldsym != ob->ob_newsym )
                        || ( ob->ob_y != ob->ob_oldy )
                        || ( ( ob->ob_oldx + displx ) != ob->ob_x ) ) {
                        if ( ob->ob_delflg )
                                ( *drawsym )( ob, ob->ob_oldx, ob->ob_oldy,
                                              ob->ob_oldsym, ob->ob_clr, NULL );
                        if ( !ob->ob_drwflg )
                                continue;
                        if ( ( ob->ob_x < displx ) || ( ob->ob_x > disprx ) ) {
                                ob->ob_drwflg = 0;
                                continue;
                        }
                        ( *drawsym )( ob, ob->ob_oldx = ob->ob_x - displx,
                                      ob->ob_oldy = ob->ob_y,
                                      ob->ob_newsym,
                                      ob->ob_clr, NULL );
                }
                if ( ob->ob_drawf )
                        ( *( ob->ob_drawf ) )( ob );
        }

        for ( ob = deltop; ob; ob = ob->ob_next )
                if ( ob->ob_delflg )
                        ( *drawsym )( ob, ob->ob_oldx, ob->ob_oldy,
                                      ob->ob_oldsym, ob->ob_clr, NULL );

        dispgrnd();
        dispinit = FALSE;
        forcdisp = FALSE;
}



/*---------------------------------------------------------------------------

        Update display of ground.   Delete previous display of ground by
        XOR graphics.

        Different routines are used to display/delete ground on colour
        or monochrome systems.

---------------------------------------------------------------------------*/




static  dispgrnd()
{
        if ( !dispinit ) {
                if ( !( dispdx || forcdisp ) )
                        return;
                ( *dispg )( grndsave );
        }
        movmem( ground + displx, grndsave, SCR_WDTH * sizeof( GRNDTYPE ) );
        ( *dispg )( ground + displx );
}




static  dispgm( gptr )
GRNDTYPE *gptr;
{
register GRNDTYPE *g, gl, gc;
register int      gmask, i;
register char     *sptr;

        i = SCR_WDTH;
        gl = *( g = gptr );
        gmask = 0xC0;
        sptr = dispoff + ( SCR_HGHT - gl - 1 ) * 160;

        while ( i-- ) {
                if ( gl == ( gc = *g++ ) ) {
                        *sptr        ^= gmask;
                        *( sptr+80 ) ^= gmask;
                } else if ( gl < gc )
                        do  {
                                *( sptr-=160 ) ^= gmask;
                                *( sptr+80 )   ^= gmask;
                        } while ( ++gl < gc );
                else
                        do  {
                                *( sptr+=160 ) ^= gmask;
                                *( sptr-80 )   ^= gmask;
                        } while ( --gl > gc );

                if ( !( gmask >>= 2 ) ) {
                        gmask = 0xC0;
                        ++sptr;
                }
        }
}




static  dispgc( gptr )
GRNDTYPE *gptr;
{
register GRNDTYPE *g, gl, gc;
register int      gmask, i;
register char     *sptr;

        i = SCR_WDTH;
        gl = *( g = gptr );
        gmask = 0x80;
        sptr = dispoff + ( SCR_HGHT - gl - 1 ) * 160;

        while ( i-- ) {
                if ( gl == ( gc = *g++ ) ) {
                        *sptr       ^= gmask;
                        *( sptr+2 ) ^= gmask;
                        *( sptr+4 ) ^= gmask;
                } else if ( gl < gc )
                        do  {
                                *( sptr-=160 ) ^= gmask;
                                *( sptr+2 )    ^= gmask;
                                *( sptr+4 )    ^= gmask;
                        } while ( ++gl < gc );
                else
                        do  {
                                *( sptr+=160 ) ^= gmask;
                                *( sptr+2 )    ^= gmask;
                                *( sptr+4 )    ^= gmask;
                        } while ( --gl > gc );

                if ( !( gmask >>= 1 ) ) {
                        gmask = 0x80;
                        if ( (long) sptr & 1 )
                                sptr += 7;
                        else
                                ++sptr;
                }
        }
}



/*---------------------------------------------------------------------------

        External display ground call for title screen processing.

---------------------------------------------------------------------------*/




swground()
{
        dispgrnd();
}



/*---------------------------------------------------------------------------

        Clear the collision detection portion of auxiliary video ram

---------------------------------------------------------------------------*/




swclrcol()
{
register long   *sptr;
register int    l;

        sptr = dispoff + ( SCR_HGHT - 1 ) * 160;
        if ( scrtype == 2 )
                for ( l = 32; l; --l ) {
                        *sptr = *( sptr + 1 ) = *( sptr + 2 ) = 0L;
                        sptr -= 20;
                }
        else
                for ( l = 16; l; --l ) {
                        *sptr = *( sptr + 1 ) = *( sptr + 2 ) = *( sptr + 3 )
                              = *( sptr + 4 ) = *( sptr + 5 ) = 0L;
                        sptr -= 40;
                }
}



/*---------------------------------------------------------------------------

        Display an object's current symbol at a specified screen location
        Collision detection may or may not be asked for.

        Different routines are used to display symbols on colour or
        monochrome systems.

---------------------------------------------------------------------------*/




swputsym( x, y, ob )
int     x, y;
OBJECTS *ob;
{
        ( *drawsym )( ob, x, y, ob->ob_newsym, ob->ob_clr, NULL );
}



swputcol( x, y, ob )
int     x, y;
OBJECTS *ob;
{
int     retcode = FALSE;

        ( *drawsym )( ob, x, y, ob->ob_newsym, ob->ob_clr, &retcode );
        return( retcode );
}




char    fill[] = {
0x00,0x03,0x03,0x03,0x0C,0x0F,0x0F,0x0F,0x0C,0x0F,0x0F,0x0F,0x0C,0x0F,0x0F,0x0F,
0x30,0x33,0x33,0x33,0x3C,0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3F,
0x30,0x33,0x33,0x33,0x3C,0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3F,
0x30,0x33,0x33,0x33,0x3C,0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3F,
0xC0,0xC3,0xC3,0xC3,0xCC,0xCF,0xCF,0xCF,0xCC,0xCF,0xCF,0xCF,0xCC,0xCF,0xCF,0xCF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xC0,0xC3,0xC3,0xC3,0xCC,0xCF,0xCF,0xCF,0xCC,0xCF,0xCF,0xCF,0xCC,0xCF,0xCF,0xCF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xC0,0xC3,0xC3,0xC3,0xCC,0xCF,0xCF,0xCF,0xCC,0xCF,0xCF,0xCF,0xCC,0xCF,0xCF,0xCF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,
0xF0,0xF3,0xF3,0xF3,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF
};




static  drawsm( ob, x, y, symbol, clr, retcode )
OBJECTS *ob;
int     x, y, clr, *retcode;
char    *symbol;
{
register char   *s, *sptr, *sym;
register int    j, c, cr, pc;
int             rotr, rotl, wdth, wrap, n;

        if ( !( sym = symbol ) )
                return;

        if ( ( ob->ob_symhgt == 1 ) && ( ob->ob_symwdt == 1 ) ) {
                drawpm( x, y, (int) sym, retcode );
                return;
        }

        rotr = ( x & 0x0003 ) << 1;
        rotl = 8 - rotr;

        if ( ( wrap = ( wdth = ob->ob_symwdt >> 2 )
                - ( n = SCR_LINW - ( x >> 2 ) ) ) > 0 )
                wdth = n;

        if ( ( n = ob->ob_symhgt ) > ( y + 1 ) )
                n = y + 1;
        sptr = dispoff + ( ( SCR_HGHT - y - 1 ) * 160 ) + ( x >> 2 );

        while ( n-- ) {
                s = sptr;
                j = wdth;
                pc = 0;
                while ( j-- ) {
                        cr = ( c = *sym++ ) << rotl;
                        c = ( ( c & 0x00FF ) >> rotr ) | pc;
                        pc = cr;
                        if ( retcode && ( *s & fill[c & 0x00FF] ) ){
                                *retcode = TRUE;
                                retcode = 0;
                        }
                        *s ^= c;
                        *((s++)+80) ^= c;
                }
                if ( wrap >= 0 )
                        sym += wrap;
                else {
                        if ( retcode && ( *s & fill[pc & 0x00FF ] ) ){
                                *retcode = TRUE;
                                retcode = 0;
                        }
                        *s      ^= pc;
                        *(s+80) ^= pc;
                }
                sptr += 160;
        }
}




static  drawsc( ob, x, y, symbol, clr, retcode )
OBJECTS *ob;
int     x, y, clr, *retcode;
char    *symbol;
{
register char   *s, *sptr, *sym;
register int    j, c1, c2, c;
int             rotr, rotl, wdth, wrap, n;
int             cr, pc1, pc2, invert, enhance1;
extern  char    swbrdsym[BIRDSYMS][BRDBYTES];

        if ( !( sym = symbol ) )
                return;

        if ( ( ob->ob_symhgt == 1 ) && ( ob->ob_symwdt == 1 ) ) {
                drawpc( x, y, (int) sym, retcode );
                return;
        }

        rotr = x & 0x0007;
        rotl = 8 - rotr;

        if ( ( wrap = ( wdth = ob->ob_symwdt >> 2 )
                - ( n = SCR_LINW - ( x >> 2 ) ) ) > 0 )
                wdth = n;

        if ( ( n = ob->ob_symhgt ) > ( y + 1 ) )
                n = y + 1;
        sptr = dispoff + ( ( SCR_HGHT - y - 1 ) * 160 )
                       + ( ( x & 0xFFF0 ) >> 1 )
                       + ( ( x & 0x0008 ) >> 3 );

        invert = ( clr & 0x0003 ) == 2 ? -1 : 0;
        enhance1 = ( ( ( j = ob->ob_type ) == FLOCK ) || ( j == BIRD )
                   || ( j == OX ) ) ? -1 : 0;
        if ( j == BIRD )
                sym = (char *) spcbirds + ( ( sym - (char *) swbrdsym ) << 1 );

        while ( n-- ) {
                s = sptr;
                j = wdth;
                pc1 = pc2 = 0;
                while ( j-- ) {
                        if ( j ) {
                                c = 0xFF;
                                --j;
                        } else
                                c = 0xF0;

                        cr = ( c1 = *sym++ & c ) << rotl;
                        c1 = ( c1 >> rotr ) | pc1;
                        pc1 = cr;
                        cr = ( c2 = *sym++ & c ) << rotl;
                        c2 = ( c2 >> rotr ) | pc2;
                        pc2 = cr;
                        c = c1 | c2;

                        if ( retcode
                                && ( c & ( *s | *(s+2) ) & 0xFF ) ) {
                                *retcode = TRUE;
                                retcode = 0;
                        }
                        *s     ^= c1 ^ ( c & invert );
                        *(s+2) ^= c2 ^ ( c & invert );
                        *(s+6) ^= c & enhance1;

                        if ( (long) s & 1 )
                                s += 7;
                        else
                                ++s;
                }
                if ( wrap >= 0 )
                        sym += wrap & 0xFFFE;
                else {
                        c = pc1 | pc2;
                        if ( retcode
                                && ( c & ( *s | *(s+2) ) & 0xFF ) ){
                                *retcode = TRUE;
                                retcode = 0;
                        }
                        *s     ^= pc1 ^ ( c & invert );
                        *(s+2) ^= pc2 ^ ( c & invert );
                        *(s+6) ^= c & enhance1;
                }
                sptr += 160;
        }
}



/*---------------------------------------------------------------------------

        External calls to display a point of a specified colour at a
        specified position.   The point request may or may not ask for
        collision detection by returning the old colour of the point.

        Different routines are used to display points on colour or
        monochrome systems.

---------------------------------------------------------------------------*/




swpntsym( x, y, clr )
int     x, y, clr;
{
        ( *drawpnt )( x, y, clr, NULL );
}



swpntcol( x, y, clr )
int     x, y, clr;
{
int     oldclr;

        ( *drawpnt )( x, y, clr, &oldclr );
        return( oldclr );
}



drawpc( x, y, clr, oldclr )
int     x, y, clr, *oldclr;
{
register  int   c, mask;
register  char  *sptr;

        sptr = dispoff + ( ( SCR_HGHT - y - 1 ) * 160 )
                       + ( ( x & 0xFFF0 ) >> 1 )
                       + ( ( x & 0x0008 ) >> 3 );
        mask = 0x80 >> ( x &= 0x0007 );

        if ( oldclr ) {
                c = ( *sptr & mask )
                        | ( ( *( sptr+2 ) & mask ) << 1 )
                        | ( ( *( sptr+4 ) & mask ) << 2 )
                        | ( ( *( sptr+6 ) & mask ) << 3 );
                *oldclr = ( c >> ( 7 - x ) ) & 0x00FF;
        }

        c = clr << ( 7 - x );
        if ( clr & 0x0080 ) {
                *sptr       ^= ( mask & c );
                *( sptr+2 ) ^= ( mask & ( c >> 1 ) );
                *( sptr+4 ) ^= ( mask & ( c >> 2 ) );
                *( sptr+6 ) ^= ( mask & ( c >> 3 ) );
        } else {
                mask = ~mask;
                *sptr       &= mask;
                *( sptr+2 ) &= mask;
                *( sptr+4 ) &= mask;
                *( sptr+6 ) &= mask;

                mask = ~mask;
                *sptr       |= ( mask & c );
                *( sptr+2 ) |= ( mask & ( c >> 1 ) );
                *( sptr+4 ) |= ( mask & ( c >> 2 ) );
                *( sptr+6 ) |= ( mask & ( c >> 3 ) );
        }
}





drawpm( x, y, clr, oldclr )
int     x, y, clr, *oldclr;
{
register  int   c, mask;
register  char  *sptr;

        sptr = dispoff + ( ( SCR_HGHT - y - 1 ) * 160 ) + ( x >> 2 );
        mask = 0xC0 >> ( x = ( x & 0x0003 ) << 1 );

        if ( oldclr )
                *oldclr = ( ( *sptr & mask ) >> ( 6 - x ) ) & 0x00FF;

        c = clr << ( 6 - x );
        if ( clr & 0x0080 ) {
                *sptr        ^= ( mask & c );
                *( sptr+80 ) ^= ( mask & c );
        } else {
                *sptr        &= ~mask;
                *( sptr+80 ) &= ~mask;
                *sptr        |= ( mask & c );
                *( sptr+80 ) |= ( mask & c );
        }
}




/*---------------------------------------------------------------------------

        Get/set the current screen resolution.  The resolution is never
        changed on a monochrome system.  Low-res 16 colour is used on
        colour systems, (equivalent to IBM type 4), except in debugging
        instances where high-res 4 colour is used. (equivalent to IBM type 6)

        On colour systems, the pixel map for each symbol is converted to
        optomize video ram updates.  The bit pattern abcdefghijklmnop is
        converted to bdfhjlnpacegikmo for all words of all symbols.
---------------------------------------------------------------------------*/




get_type()
{
        return( scrtype = trap14( 4 ) );
}



set_type( type )
int     type;
{
        if ( type > 2 ) {
                if ( scrtype == 2 ) {
                        type = 2;
                        dispg = dispgm;
                        drawpnt = drawpm;
                        drawsym = drawsm;
                } else {
                        if ( type == 6 )
                                type = 1;
                        else
                                type = 0;
                        dispg = dispgc;
                        drawpnt = drawpc;
                        drawsym = drawsc;
                        invertsymbols();
                }
                trap14( 5, -1L, -1L, type );
                trap14( 6, palette );
        } else {
                trap14( 5, -1L, -1L, type );
                trap14( 21, 1 );
        }
}



static  invertsymbols()
{
extern  char    swplnsym[ORIENTS][ANGLES][SYMBYTES];
extern  char    swhitsym[HITSYMS][SYMBYTES];
extern  char    swbmbsym[BOMBANGS][BOMBBYTES];
extern  char    swtrgsym[TARGORIENTS][TARGBYTES];
extern  char    swwinsym[WINSIZES][WINBYTES];
extern  char    swhtrsym[TARGBYTES];
extern  char    swexpsym[EXPLSYMS][EXPBYTES];
extern  char    swflksym[FLCKSYMS][FLKBYTES];
extern  char    swbrdsym[BIRDSYMS][BRDBYTES];
extern  char    swoxsym[OXSYMS][OXBYTES];

        invert( swplnsym, ORIENTS*ANGLES*SYMBYTES );
        invert( swhitsym, HITSYMS*SYMBYTES        );
        invert( swbmbsym, BOMBANGS*BOMBBYTES      );
        invert( swtrgsym, TARGORIENTS*TARGBYTES   );
        invert( swwinsym, WINSIZES*WINBYTES       );
        invert( swhtrsym, TARGBYTES               );
        invert( swexpsym, EXPLSYMS*EXPBYTES       );
        invert( swflksym, FLCKSYMS*FLKBYTES       );
        copy( swbrdsym, spcbirds );
        invert( spcbirds, BIRDSYMS*BRDBYTES*2     );
        invert( swoxsym,  OXSYMS*OXBYTES          );
}



static  copy( from, to )
char    *from, *to;
{
int     i;

        for ( i = 4; i; --i ) {
                *to++ = *from++;
                *to++ = '\0';
        }
}



static  invert( symbol, bytes )
char    *symbol;
int     bytes;
{
register int    c1, c2;
register char   *s;
int             n;

        s = symbol;
        for ( n = bytes >> 1; n; --n ) {
                c1 = *s;
                c2 = *( s + 1 );
                *s++ =  ( ( c1 << 1 ) & 0x80 )
                        | ( ( c1 << 2 ) & 0x40 )
                        | ( ( c1 << 3 ) & 0x20 )
                        | ( ( c1 << 4 ) & 0x10 )
                        | ( ( c2 >> 3 ) & 0x08 )
                        | ( ( c2 >> 2 ) & 0x04 )
                        | ( ( c2 >> 1 ) & 0x02 )
                        | ( c2 & 0x01 );
                *s++ =  ( c1 & 0x80 )
                        | ( ( c1 << 1 ) & 0x40 )
                        | ( ( c1 << 2 ) & 0x20 )
                        | ( ( c1 << 3 ) & 0x10 )
                        | ( ( c2 >> 4 ) & 0x08 )
                        | ( ( c2 >> 3 ) & 0x04 )
                        | ( ( c2 >> 2 ) & 0x02 )
                        | ( ( c2 >> 1 ) & 0x01 );
        }
}






/*---------------------------------------------------------------------------

        External calls to specify current video ram as screen ram or
        auxiliary screen area.

---------------------------------------------------------------------------*/




setvdisp()
{
static  char    *videoram = NULL;

        if ( !videoram )
                videoram = trap14( 3 );
        dispoff = videoram;
}




setadisp()
{
        dispoff = auxdisp - 0x4000;
}
