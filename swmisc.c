/*
        swmiscjr -      SW miscellaneous

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-07-23        Development
                        87-03-09        Microsoft compiler.
*/

#include        "sw.h"
#include        "font.h"

// sdh: emulate text display

static int cur_x=0, cur_y=0;                  // place we are writing text
static int cur_color;                         // text color

static inline void drawchar(int x, int y, int c)
{
	unsigned char *p;
	int x2, y2;
	
	if(c < 0 || c >= 256)
		return;

	p = font_data + c * 8;

	for(y2=0; y2<8; ++y2) {
		int m = 0x80;

		for(x2=0; x2<8; ++x2) {
			if(p[y2] & m) {
				swpntsym(x+x2, SCR_HGHT - (y+y2), cur_color);
			}
			m >>= 1;
		}
	}
}

swputs( sp )
char    *sp;
{
	register char   *s;

	setvdisp();
	
	for(s=sp; *s; ++s) {
		if(isprint(*s))
			drawchar(cur_x * 8, cur_y * 8, *s);
		++cur_x;
		if(*s == '\n') {
			cur_x = 0;
			++cur_y;
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
