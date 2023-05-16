//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
//        swtext - text processing.. input/output
//

#include <ctype.h>
#include <string.h>

#include "font.h"
#include "timer.h"
#include "video.h"

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

	if (c < 0 || c >= 256) {
		return;
	}

	p = font_data + c * 8;

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

void swputs(const char *sp)
{
	const char *s;

	for (s = sp; *s; ++s) {
		swputc(*s);
	}
}

void swgets(char *s, int max)
{
	int or_x = cur_x, or_y = cur_y;
	int erase_len = 0;
	int x, y;

	Vid_StartTextInput();

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
			if (strlen(s) > 0) {
				s[strlen(s) - 1] = '\0';
			}
		} else if (c == '\n') {
			break;
		}
	}

	Vid_StopTextInput();
}

void swcolor(int a)
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
	bool first = true;

	if (n < 0) {
		n = -n;
		swputc('-');
		++i;
	}
	for (t = 10000; t > 1; n %= t, t /= 10) {
		d = n / t;
		if (d || !first) {
			first = false;
			swputc(d + '0');
			++i;
		}
	}
	swputc(n + '0');
	++i;
	while (++i <= size) {
		swputc(' ');
	}
}

int swgetc(void)
{
	int i;

	while(!(i = Vid_GetChar())) {

		// sdh 15/11/2001: dont thrash the processor while
		// waiting for a key press
		Timer_Sleep(50);

		swsndupdate();
		if (ctlbreak())
			break;
	}

	return i;
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
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
