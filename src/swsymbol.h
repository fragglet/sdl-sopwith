//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// You can redistribute and/or modify this program under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation, or any later version. This program is distributed WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
//
//
// Sprites
//

#ifndef __SWSYMBOL_H__
#define __SWSYMBOL_H__

typedef struct sopsym_s sopsym_t;
typedef struct symset_s symset_t;

#include "sw.h"

struct sopsym_s {
	unsigned char *data;
	int w, h;
};

// All possible rotations of sym[0]:
struct symset_s {
	sopsym_t sym[8];
	const char *name;
	int frame;
	symset_t *next;
};

extern symset_t *all_symsets;

extern symset_t symbol_bomb[2];                 // swbmbsym
extern symset_t symbol_targets[4];              // swtrgsym
extern symset_t symbol_target_hit[1];           // swhtrsym
extern symset_t symbol_debris[8];               // swexpsym
extern symset_t symbol_flock[2];                // swflksym
extern symset_t symbol_bird[2];                 // swbrdsym
extern symset_t symbol_ox[2];                   // swoxsym
extern symset_t symbol_shotwin[1];              // swshtsym
extern symset_t symbol_birdsplat[1];            // swsplsym
extern symset_t symbol_missile[4];              // swmscsym
extern symset_t symbol_burst[2];                // swbstsym
extern symset_t symbol_plane[4];                // swplnsym
extern symset_t symbol_plane_hit[2];            // swhitsym
extern symset_t symbol_plane_win[4];            // swwinsym
extern symset_t symbol_medal[3];                // swmedalsym
extern symset_t symbol_ribbon[6];               // swribbonsym

extern sopsym_t symbol_pixel;

extern void symbol_generate(void);
extern void symset_from_text(symset_t *s, const char *text, int w, int h);
symset_t *lookup_symset(const char *name, int frame);

#endif

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/06/2002: move plane symbol headers here
// sdh 27/06/2002: add sopsym_t, sopsym_t sprite frame replacements
// sdh 21/10/2001: moved plane sprite constants into here from sw.h
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
