// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001 Simon Howard
//
// All rights reserved except as specified in the file license.txt.
// Distribution of this file without the license.txt file accompanying
// is prohibited.
//
//---------------------------------------------------------------------------
//
//        swgrph   -      SW screen graphics
//
//---------------------------------------------------------------------------

#include "cgavideo.h"

#include "sw.h"
#include "swdisp.h"
#include "swground.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swplanes.h"
#include "swsymbol.h"
#include "swutil.h"

//#define SOLID_GROUND

static char *dispoff;		        /* Current display offset            */
static int scrtype;		        /* Screen type                       */
static GRNDTYPE grndsave[SCR_WDTH];	/* Saved ground buffer for last      */
					/*   last display                    */
static void (*dispg) ();	        /* display ground routine (mono,clr) */
static void (*drawpnt) ();              /* draw point routine                */
static void (*drawsym) ();              /* draw symbol routine               */

static int palette[] = {        /* Colour palette                            */
	0x000,	                /*   0 = black    background                 */
	0x037,		        /*   1 = blue     planes,targets,explosions  */
	0x700,		        /*   2 = red      planes,targets,explosions  */
	0x777,			/*   3 = white    bullets                    */
	0x000,			/*   4                                       */
	0x000,			/*   5                                       */
	0x000,			/*   6                                       */
	0x070,			/*   7 = green    ground                     */
	0x000,			/*   8                                       */
	0x433,			/*   9 = tan      oxen, birds                */
	0x420,			/*  10 = brown    oxen                       */
	0x320,			/*  11 = brown    bottom of ground display   */
	0x000,			/*  12                                       */
	0x000,			/*  13                                       */
	0x000,			/*  14                                       */
	0x000			/*  15                                       */
};



static char spcbirds[BIRDSYMS][BRDBYTES * 2];	/* Special bird symbol    */
						/* colour video maps      */

// sdh 28/10/2001: moved auxdisp here

char    auxdisp[VRAMSIZE];


/*---------------------------------------------------------------------------

        Update display of ground.   Delete previous display of ground by
        XOR graphics.

        Different routines are used to display/delete ground on colour
        or monochrome systems.

---------------------------------------------------------------------------*/




static void dispgrnd()
{
	if (!dispinit) {
		if (!(dispdx || forcdisp))
			return;
		(*dispg) (grndsave);
	}

	// sdh 16/10/2001: removed movmem

	memcpy(grndsave, ground+displx, SCR_WDTH * sizeof(GRNDTYPE));

	(*dispg) (ground + displx);
}




static void dispgm(GRNDTYPE * gptr)
{
	register GRNDTYPE *g = gptr, gl, gc;
	register int gmask, i;
	register char *sptr;

	i = SCR_WDTH;
	gl = *g;
	gmask = 0xC0;
	sptr = dispoff + (SCR_HGHT - gl - 1) * 160;

	while (i--) {
		gc = *g++;

		if (gl == gc) {
			*sptr ^= gmask;
			*(sptr + 80) ^= gmask;
		} else if (gl < gc)
			do {
				*(sptr -= 160) ^= gmask;
				*(sptr + 80) ^= gmask;
			} while (++gl < gc);
		else
			do {
				*(sptr += 160) ^= gmask;
				*(sptr - 80) ^= gmask;
			} while (--gl > gc);

		if (!(gmask >>= 2)) {
			gmask = 0xC0;
			++sptr;
		}
	}
}



static void dispgc(GRNDTYPE * gptr)
{
	register GRNDTYPE *g = gptr, gl, gc;
	register int gmask, i;
	register char *sptr;

	i = SCR_WDTH;
	gl = *g;
	gmask = 0x80;
	sptr = dispoff + (SCR_HGHT - gl - 1) * 160;

	while (i--) {
		gc = *g++;
		if (gl == gc) {
			*sptr ^= gmask;
			*(sptr + 2) ^= gmask;
			*(sptr + 4) ^= gmask;
		} else if (gl < gc)
			do {
				*(sptr -= 160) ^= gmask;
				*(sptr + 2) ^= gmask;
				*(sptr + 4) ^= gmask;
			} while (++gl < gc);
		else
			do {
				*(sptr += 160) ^= gmask;
				*(sptr + 2) ^= gmask;
				*(sptr + 4) ^= gmask;
			} while (--gl > gc);

		if (!(gmask >>= 1)) {
			gmask = 0x80;
			if ((long) sptr & 1)
				sptr += 7;
			else
				++sptr;
		}
	}
}


// sdh 28/10/2001: solid ground function

static void dispgc_solid(GRNDTYPE * gptr)
{
	register GRNDTYPE *g = gptr, gl, gc;
	register int gmask, i;
	register char *sptr;
	register int sptr_offset = 0;
		
	i = SCR_WDTH;
	gl = *g;
	gmask = 0x80;
	sptr = dispoff + (SCR_HGHT - gl - 1) * 160;

	while (i--) {
		gc = *g++;
		sptr = dispoff + (SCR_HGHT - gc - 2) * 160 + sptr_offset;

		// sdh 21/10/2001: solid ground like in sopwith 1
		do {
			*( sptr += 160) ^= gmask;
			sptr[2] ^= gmask;
			sptr[4] ^= gmask;
		} while(--gc > 18);

		if (!(gmask >>= 1)) {
			gmask = 0x80;
			if ((long) sptr_offset & 1)
				sptr_offset += 7;
			else
				++sptr_offset;
		}
	}
}



/*---------------------------------------------------------------------------

        External display ground call for title screen processing.

---------------------------------------------------------------------------*/




void swground()
{
	dispgrnd();
}



/*---------------------------------------------------------------------------

        Clear the collision detection portion of auxiliary video ram

---------------------------------------------------------------------------*/




void swclrcol()
{
	register long *sptr;
	register int l;

	sptr = (long *) (dispoff + (SCR_HGHT - 1) * 160);
	if (scrtype == 2)
		for (l = 32; l; --l) {
			*sptr = *(sptr + 1) = *(sptr + 2) = 0L;
			sptr -= 20;
	} else {
		for (l = 16; l; --l) {
			*sptr = *(sptr + 1) = *(sptr + 2) = *(sptr + 3)
			    = *(sptr + 4) = *(sptr + 5) = 0L;
			sptr -= 40;
		}
	}
}


/*---------------------------------------------------------------------------

        External calls to display a point of a specified colour at a
        specified position.   The point request may or may not ask for
        collision detection by returning the old colour of the point.

        Different routines are used to display points on colour or
        monochrome systems.

---------------------------------------------------------------------------*/




void swpntsym(int x, int y, int clr)
{
	(*drawpnt) (x, y, clr, NULL);
}



int swpntcol(int x, int y, int clr)
{
	int oldclr;

	(*drawpnt) (x, y, clr, &oldclr);

	return oldclr;
}



static void drawpc(int x, int y, int clr, int *oldclr)
{
	register int c, mask;
	register char *sptr;

	sptr = dispoff 
	       + (SCR_HGHT - y - 1) * 160
	       + ((x & 0xFFF0) >> 1)
	       + ((x & 0x0008) >> 3);
	x &= 0x0007;
	mask = 0x80 >> x;

	if (oldclr) {
		c = (*sptr & mask)
		    | ((*(sptr + 2) & mask) << 1)
		    | ((*(sptr + 4) & mask) << 2)
		    | ((*(sptr + 6) & mask) << 3);
		*oldclr = (c >> (7 - x)) & 0x00FF;
	}

	c = clr << (7 - x);
	if (clr & 0x0080) {
		*sptr ^= (mask & c);
		*(sptr + 2) ^= (mask & (c >> 1));
		*(sptr + 4) ^= (mask & (c >> 2));
		*(sptr + 6) ^= (mask & (c >> 3));
	} else {
		mask = ~mask;
		*sptr &= mask;
		*(sptr + 2) &= mask;
		*(sptr + 4) &= mask;
		*(sptr + 6) &= mask;

		mask = ~mask;
		*sptr |= (mask & c);
		*(sptr + 2) |= (mask & (c >> 1));
		*(sptr + 4) |= (mask & (c >> 2));
		*(sptr + 6) |= (mask & (c >> 3));
	}
}





static void drawpm(int x, int y, int clr, int *oldclr)
{
	register int c, mask;
	register char *sptr;

	sptr = dispoff + ((SCR_HGHT - y - 1) * 160) + (x >> 2);
	x = (x & 0x0003) << 1;
	mask = 0xC0 >> x;

	if (oldclr)
		*oldclr = ((*sptr & mask) >> (6 - x)) & 0x00FF;

	c = clr << (6 - x);
	if (clr & 0x0080) {
		*sptr ^= (mask & c);
		*(sptr + 80) ^= (mask & c);
	} else {
		*sptr &= ~mask;
		*(sptr + 80) &= ~mask;
		*sptr |= (mask & c);
		*(sptr + 80) |= (mask & c);
	}
}



/*---------------------------------------------------------------------------

        Display an object's current symbol at a specified screen location
        Collision detection may or may not be asked for.

        Different routines are used to display symbols on colour or
        monochrome systems.

---------------------------------------------------------------------------*/




void swputsym(int x, int y, OBJECTS * ob)
{
	(*drawsym) (ob, x, y, ob->ob_newsym, ob->ob_clr, NULL);
}



int swputcol(int x, int y, OBJECTS * ob)
{
	int retcode = FALSE;

	(*drawsym) (ob, x, y, ob->ob_newsym, ob->ob_clr, &retcode);

	return retcode;
}




char fill[] = {
	0x00, 0x03, 0x03, 0x03, 0x0C, 0x0F, 0x0F, 0x0F, 0x0C, 0x0F, 0x0F,
	    0x0F, 0x0C, 0x0F, 0x0F, 0x0F,
	0x30, 0x33, 0x33, 0x33, 0x3C, 0x3F, 0x3F, 0x3F, 0x3C, 0x3F, 0x3F,
	    0x3F, 0x3C, 0x3F, 0x3F, 0x3F,
	0x30, 0x33, 0x33, 0x33, 0x3C, 0x3F, 0x3F, 0x3F, 0x3C, 0x3F, 0x3F,
	    0x3F, 0x3C, 0x3F, 0x3F, 0x3F,
	0x30, 0x33, 0x33, 0x33, 0x3C, 0x3F, 0x3F, 0x3F, 0x3C, 0x3F, 0x3F,
	    0x3F, 0x3C, 0x3F, 0x3F, 0x3F,
	0xC0, 0xC3, 0xC3, 0xC3, 0xCC, 0xCF, 0xCF, 0xCF, 0xCC, 0xCF, 0xCF,
	    0xCF, 0xCC, 0xCF, 0xCF, 0xCF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xC0, 0xC3, 0xC3, 0xC3, 0xCC, 0xCF, 0xCF, 0xCF, 0xCC, 0xCF, 0xCF,
	    0xCF, 0xCC, 0xCF, 0xCF, 0xCF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xC0, 0xC3, 0xC3, 0xC3, 0xCC, 0xCF, 0xCF, 0xCF, 0xCC, 0xCF, 0xCF,
	    0xCF, 0xCC, 0xCF, 0xCF, 0xCF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF3, 0xF3, 0xF3, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF,
	    0xFF, 0xFC, 0xFF, 0xFF, 0xFF
};




static void drawsm(OBJECTS * ob, int x, int y,
		   char *symbol, int clr, int *retcode)
{
	register char *s, *sptr, *sym;
	register int j, c, cr, pc;
	int rotr, rotl, wdth, wrap, n;

	if (!symbol)
		return;

	sym = symbol;

	if (ob->ob_symhgt == 1 && ob->ob_symwdt == 1) {
		drawpm(x, y, (int) sym, retcode);
		return;
	}

	rotr = (x & 0x0003) << 1;
	rotl = 8 - rotr;

	wdth = ob->ob_symwdt >> 2;
	n = SCR_LINW - (x >> 2);
	wrap = wdth - n;
 
	if (wrap > 0)
		wdth = n;

	n = ob->ob_symhgt;
	if (n > (y + 1))
		n = y + 1;
	sptr = dispoff + ((SCR_HGHT - y - 1) * 160) + (x >> 2);

	while (n--) {
		s = sptr;
		j = wdth;
		pc = 0;
		while (j--) {
			cr = (c = *sym++) << rotl;
			c = ((c & 0x00FF) >> rotr) | pc;
			pc = cr;
			if (retcode && (*s & fill[c & 0x00FF])) {
				*retcode = TRUE;
				retcode = 0;
			}
			*s ^= c;
			*((s++) + 80) ^= c;
		}
		if (wrap >= 0)
			sym += wrap;
		else {
			if (retcode && (*s & fill[pc & 0x00FF])) {
				*retcode = TRUE;
				retcode = 0;
			}
			*s ^= pc;
			*(s + 80) ^= pc;
		}
		sptr += 160;
	}
}




static void drawsc(OBJECTS * ob, int x, int y,
		   char *symbol, int clr, int *retcode)
{
	register char *s, *sptr, *sym;
	register int j, c1, c2, c;
	int rotr, rotl, wdth, wrap, n;
	int cr, pc1, pc2, invert, enhance1;
	obtype_t obtype;

	if (!symbol)
		return;

	sym = symbol;

	if (ob->ob_symhgt == 1 && ob->ob_symwdt == 1) {
		drawpc(x, y, (int) sym, retcode);
		return;
	}

	rotr = x & 0x0007;
	rotl = 8 - rotr;

	wdth = ob->ob_symwdt >> 2;
	n = SCR_LINW - (x >> 2);
	wrap = wdth - n;

	if (wrap > 0)
		wdth = n;

	n = ob->ob_symhgt;

	if (n > (y + 1))
		n = y + 1;
	sptr = dispoff + ((SCR_HGHT - y - 1) * 160)
	    + ((x & 0xFFF0) >> 1)
	    + ((x & 0x0008) >> 3);

	invert = (clr & 0x0003) == 2 ? -1 : 0;

	obtype = ob->ob_type;

	enhance1 = (obtype == FLOCK || obtype == BIRD 
		    || obtype == OX) ? -1 : 0;
	if (obtype == BIRD)
		sym = (char *) spcbirds + ((sym - (char *) swbrdsym) << 1);

	while (n--) {
		s = sptr;
		j = wdth;
		pc1 = pc2 = 0;

		while (j--) {

			if (j) {
				c = 0xFF;
				--j;
			} else
				c = 0xF0;

			cr = (c1 = *sym++ & c) << rotl;
			c1 = (c1 >> rotr) | pc1;
			pc1 = cr;
			cr = (c2 = *sym++ & c) << rotl;
			c2 = (c2 >> rotr) | pc2;
			pc2 = cr;
			c = c1 | c2;

			if (retcode && (c & (*s | *(s + 2)) & 0xFF)) {
				*retcode = TRUE;
				retcode = 0;
			}

			*s ^= c1 ^ (c & invert);
			*(s + 2) ^= c2 ^ (c & invert);
			*(s + 6) ^= c & enhance1;

			if ((long) s & 1)
				s += 7;
			else
				++s;
		}

		if (wrap >= 0)
			sym += wrap & 0xFFFE;
		else {
			c = pc1 | pc2;
			if (retcode && (c & (*s | *(s + 2)) & 0xFF)) {
				*retcode = TRUE;
				retcode = 0;
			}

			*s ^= pc1 ^ (c & invert);
			*(s + 2) ^= pc2 ^ (c & invert);
			*(s + 6) ^= c & enhance1;
		}
		sptr += 160;
	}
}


/*---------------------------------------------------------------------------

        Main display loop.   Delete and display all visible objects.
        Delete any newly deleted objects

---------------------------------------------------------------------------*/



void swdisp()
{
	register OBJECTS *ob;

	setvdisp();

	for (ob = objtop; ob; ob = ob->ob_next) {
		if (!(ob->ob_delflg && ob->ob_drwflg)
		    || ob->ob_symhgt == 1
		    || ob->ob_oldsym != ob->ob_newsym
		    || ob->ob_y != ob->ob_oldy
		    || (ob->ob_oldx + displx) != ob->ob_x) {
			if (ob->ob_delflg)
				(*drawsym) (ob, ob->ob_oldx, ob->ob_oldy,
					    ob->ob_oldsym, ob->ob_clr,
					    NULL);
			if (!ob->ob_drwflg)
				continue;
			if (ob->ob_x < displx || ob->ob_x > disprx) {
				ob->ob_drwflg = 0;
				continue;
			}
			ob->ob_oldx = ob->ob_x - displx;
			ob->ob_oldy = ob->ob_y;
			(*drawsym) (ob,
				    ob->ob_oldx,
				    ob->ob_oldy,
				    ob->ob_newsym, 
				    ob->ob_clr, NULL);
		}

		if (ob->ob_drawf)
			(*(ob->ob_drawf)) (ob);
	}

	for (ob = deltop; ob; ob = ob->ob_next)
		if (ob->ob_delflg)
			(*drawsym) (ob, ob->ob_oldx, ob->ob_oldy,
				    ob->ob_oldsym, ob->ob_clr, NULL);

	dispgrnd();

	dispinit = FALSE;
	forcdisp = TRUE;

	// need to update the screen as we arent writing
	// directly into vram any more

	CGA_Update();
}


static void copy(char *from, char *to)
{
	int i;

	for (i = 4; i; --i) {
		*to++ = *from++;
		*to++ = '\0';
	}
}



static void invert(char *symbol, int bytes)
{
	register int c1, c2;
	register char *s;
	int n;

	s = symbol;
	for (n = bytes >> 1; n; --n) {
		c1 = *s;
		c2 = *(s + 1);
		*s++ = ((c1 << 1) & 0x80)
		    | ((c1 << 2) & 0x40)
		    | ((c1 << 3) & 0x20)
		    | ((c1 << 4) & 0x10)
		    | ((c2 >> 3) & 0x08)
		    | ((c2 >> 2) & 0x04)
		    | ((c2 >> 1) & 0x02)
		    | (c2 & 0x01);
		*s++ = (c1 & 0x80)
		    | ((c1 << 1) & 0x40)
		    | ((c1 << 2) & 0x20)
		    | ((c1 << 3) & 0x10)
		    | ((c2 >> 4) & 0x08)
		    | ((c2 >> 3) & 0x04)
		    | ((c2 >> 2) & 0x02)
		    | ((c2 >> 1) & 0x01);
	}
}

static void invertsymbols()
{
	// sdh 28/10/2001: dont invert more than once

	static BOOL inverted = FALSE;

	if (inverted)
		return;

	invert((char *) swplnsym, ORIENTS * ANGLES * SYMBYTES);
	invert((char *) swhitsym, HITSYMS * SYMBYTES);
	invert((char *) swbmbsym, BOMBANGS * BOMBBYTES);
	invert((char *) swtrgsym, TARGORIENTS * TARGBYTES);
	invert((char *) swwinsym, WINSIZES * WINBYTES);
	invert((char *) swhtrsym, TARGBYTES);
	invert((char *) swexpsym, EXPLSYMS * EXPBYTES);
	invert((char *) swflksym, FLCKSYMS * FLKBYTES);
	copy((char *) swbrdsym, (char *) spcbirds);
	invert((char *) spcbirds, BIRDSYMS * BRDBYTES * 2);
	invert((char *) swoxsym, OXSYMS * OXBYTES);
	invert((char *) swshtsym, SHOTBYTES);
	invert((char *) swsplsym, SPLTBYTES);
	invert((char *) swbstsym, BRSTSYMS * BRSTBYTES);
	invert((char *) swmscsym, MISCANGS * MISCBYTES);

	inverted = TRUE;
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


// sdh 28/10/2001: removed get_type and set_type, replaced with these

void swshutdowngrph()
{
	CGA_Shutdown();
}

// sdh: just set the res using the cga (sdl) calls

void swinitgrph()
{
	CGA_Init();

	dispg = conf_solidground ? dispgc_solid : dispgc;
	drawpnt = drawpc;
	drawsym = drawsc;
	invertsymbols();

	return;
	/*
	   if (type > 2 ) {
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
	 */
}

// sdh: experiments into fixing splatted ox
// color the screen all one color

void colorscreen(int color)
{
	int x, y;

	for (y=19; y<SCR_HGHT; ++y) {
		for (x=0; x<SCR_WDTH; ++x) {
			swpntsym(x, y, color);
		}
	}
}


/*---------------------------------------------------------------------------

        External calls to specify current video ram as screen ram or
        auxiliary screen area.

---------------------------------------------------------------------------*/

static char *vidram = NULL;

void setvdisp()
{
	vidram = CGA_GetVRAM();

	dispoff = vidram;
}

void setadisp()
{
	static BOOL firstadisp = TRUE;
 
	if (firstadisp) {
		firstadisp = FALSE;
		memset(auxdisp, 0, 0x8000);
	}
	dispoff = auxdisp;

//	CGA_ClearScreen();
//	dispoff = malloc(0x4000);
//        dispoff = auxdisp - 0x4000;
}

// sdh 28/10/2001: moved various auxdisp functions here:

void movedisp()
{
#ifdef IBMPC
	swsetblk(0, SCR_SEGM, 0x1000, 0);
	swsetblk(SCR_ROFF, SCR_SEGM, 0x1000, 0);
	movblock(auxdisp, dsseg(), 0x1000, SCR_SEGM, 0x1000);
	movblock(auxdisp + 0x1000, dsseg(), 0x3000, SCR_SEGM, 0x1000);
#endif

	memset(vidram, 0, 0x8000);
	memcpy(vidram+0x7000, auxdisp+0x7000, 0x1000);
}


void clrdispv()
{
	memset(vidram, 0, VRAMSIZE);
}

void clrdispa()
{
	memset(auxdisp, 0, VRAMSIZE);
}


// sdh: screenshot function

void screendump()
{
	FILE *fs = fopen("screendump.bin", "wb");
	printf("screendump\n");

	fwrite(dispoff, VRAMSIZE, 1, fs);

	fclose(fs);
}

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 28/10/2001: get_type/set_type removed
// sdh 28/10/2001: moved auxdisp and auxdisp functions here
// sdh 24/10/2001: fix auxdisp buffer
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: added #define for solid ground (sopwith 1 style)
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed extern definitions, these are in headers now
//                 shuffled some functions round to shut up the compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-03-09        Microsoft compiler.
// 85-11-05        Atari
// 84-06-13        PCjr Speed-up
// 84-02-21        Development
//
//---------------------------------------------------------------------------

