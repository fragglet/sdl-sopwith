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
//        swmiscjr -      SW miscellaneous
//
//---------------------------------------------------------------------------

#include <ctype.h>

#include "cgavideo.h"
#include "font.h"
#include "timer.h"

#include "sw.h"
#include "swgrpha.h"
#include "swmisc.h"
#include "swsound.h"
#include "swtitle.h"

// sdh: emulate text display

static int cur_x = 0, cur_y = 0;	// place we are writing text
static int cur_color;		// text color

static inline void drawchar(int x, int y, int c)
{
	unsigned char *p;
	int x2, y2;

	if (c < 0 || c >= 256)
		return;

	p = font_data + c * 8;

	for (y2 = 0; y2 < 8; ++y2) {
		int m = 0x80;

		for (x2 = 0; x2 < 8; ++x2) {
			if (p[y2] & m) {
				// sdh 17/10/2001: -1 to y co-ordinate
				// to stop it overwriting the wrong memory

				swpntsym(x + x2,
					 SCR_HGHT - (y + y2 - 1),
					 cur_color);
			} else {
				swpntsym(x + x2,
					 SCR_HGHT - (y + y2 -1 ),
					 cur_color & 0x80);
			}
			m >>= 1;
		}
	}
}


// sdh 17/10/01: moved swputc here and made functional

void swputc(char c)
{
	if (isprint(c)) {
		drawchar(cur_x * 8, cur_y * 8, c);
	}
	++cur_x;
	if (c == '\n') {
		cur_x = 0;
		++cur_y;
	}
}

void swputs(char *sp)
{
	register char *s;

	setvdisp();

	for (s = sp; *s; ++s) {
		swputc(*s);
	}
}

// sdh 17/10/01: added swgets to read input (for reading hostnames
// in net connect)

void swgets(char *s, int max)
{
	int or_x = cur_x, or_y = cur_y;
	int erase_len = 0;
	int x, y;

	FOREVER {
		unsigned char c;

		// erase background from previous write

		for (y = 0; y < 8; ++y) {
			for (x = 0; x < erase_len * 8; ++x) {
				swpntsym(or_x * 8 + x,
					 SCR_HGHT - (or_y * 8 + y), 0);
			}
		}

		cur_x = or_x;
		cur_y = or_y;
		erase_len = strlen(s);
		swputs(s);
		CGA_Update();

		// read next keypress

		while (!(c = swgetc()));

		if (isprint(c) && strlen(s) < max) {
			s[strlen(s) + 1] = '\0';
			s[strlen(s)] = c;
		} else if (c == '\b') {
			// backspace
			s[strlen(s) - 1] = '\0';
		} else if (c == '\n') {
			break;
		}
	}
}

void swcolour(int a)
{
	cur_color = a;
}

void swposcur(int a, int b)
{
	cur_x = a;
	cur_y = b;
}

int swgetc()
{
	int i;

	while(!(i = CGA_GetKey())) {

		// sdh 15/11/2001: dont thrash the processor while 
		// waiting for a key press

		Timer_Sleep(100);

		swsndupdate();
		if (ctlbreak())
			break;
	}

	return i;
}

void swflush()
{
	// something to do with the keyboard
}


//---------------------------------------------------------------------------
//
// $Log: $
//
//
// sdh 15/11/2001: dont thrash the processor while waiting for a keypress
// sdh 24/10/2001: fix auxdisp buffer code
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent
//
// 87-03-09        Microsoft compiler.
// 84-07-23        Development
//
//---------------------------------------------------------------------------

