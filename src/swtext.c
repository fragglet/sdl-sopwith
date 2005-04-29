// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------------
//
//        swtext - text processing.. input/output
//
//---------------------------------------------------------------------------

#include <ctype.h>
#include <string.h>

#include "font.h"
#include "timer.h"
#include "video.h"

#include "sw.h"
#include "swgrpha.h"
#include "swtext.h"
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

	// sdh 27/03/02: use new drawing functions

	for (y2 = 0; y2 < 8; ++y2) {
		int m = 0x80;

		for (x2 = 0; x2 < 8; ++x2) {
			if (p[y2] & m) {
				// sdh 17/10/2001: -1 to y co-ordinate
				// to stop it overwriting the wrong memory

				Vid_PlotPixel
					(x + x2,
					 SCR_HGHT - (y + y2 - 1),
					 cur_color);
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

	for (;;) {
		unsigned char c;

		// erase background from previous write

		for (y = 0; y < 8; ++y) {
			for (x = 0; x < erase_len * 8; ++x) {
				Vid_PlotPixel
					(or_x * 8 + x,
					 SCR_HGHT - (or_y * 8 + y), 0);
			}
		}

		cur_x = or_x;
		cur_y = or_y;
		erase_len = strlen(s);
		swputs(s);
		Vid_Update();

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


void swdispd(int n, int size)
{
	int i = 0;
	int d, t;
	BOOL first = TRUE;

	// sdh 24/10/2001: make sure we use the main video buffer

	if (n < 0) {
		n = -n;
		swputc('-');
		++i;
	}
	for (t = 10000; t > 1; n %= t, t /= 10) {
		d = n / t;
		if (d || !first) {
			first = FALSE;
			swputc(d + '0');
			++i;
		}
	}
	swputc(n + '0');
	++i;
	while (++i <= size)
		swputc(' ');
}

int swgetc()
{
	int i;

	while(!(i = Vid_GetKey())) {

		// sdh 15/11/2001: dont thrash the processor while 
		// waiting for a key press

		Timer_Sleep(100);

		swsndupdate();
		if (ctlbreak())
			break;
	}

	return i;
}


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.2  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.1  2004/10/15 17:52:32  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.4  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.3  2003/06/08 03:41:41  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.2.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.2  2003/04/05 22:55:11  fraggle
// Remove the FOREVER macro and some unused stuff from std.h
//
// Revision 1.1.1.1  2003/02/14 19:03:14  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 26/03/2002: change CGA_ to Vid_
// sdh 15/11/2001: dont thrash the processor while waiting for a keypress
// sdh 24/10/2001: fix auxdisp buffer code
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent
//
// 87-03-09        Microsoft compiler.
// 84-07-23        Development
//
//---------------------------------------------------------------------------

