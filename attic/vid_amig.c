// old amiga graphics routines (I thought these were cga for
// ages!)

// unused now

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


void swclrcol()
{
	int x, y;

	for (y=0; y<16; ++y)
		for (x=0; x<48; ++x)
			(*drawpnt)(x, y, 0, NULL);

#if 0
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
#endif
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
