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
// Sprites
//
//---------------------------------------------------------------------------

#ifndef __SWSYMBOL_H__
#define __SWSYMBOL_H__

typedef struct sopsym_s sopsym_t;

#include "sw.h"

struct sopsym_s {
	unsigned char *data;
	int w, h;
};

extern sopsym_t *symbol_bomb[8];                 // swbmbsym
extern sopsym_t *symbol_targets[4];              // swtrgsym
extern sopsym_t *symbol_target_hit;              // swhtrsym
extern sopsym_t *symbol_debris[8];               // swexpsym
extern sopsym_t *symbol_flock[2];                // swflksym
extern sopsym_t *symbol_bird[2];                 // swbrdsym
extern sopsym_t *symbol_ox[2];                   // swoxsym
extern sopsym_t *symbol_ghost;                   // swghtsym
extern sopsym_t *symbol_shotwin;                 // swshtsym
extern sopsym_t *symbol_birdsplat;               // swsplsym
extern sopsym_t *symbol_missile[16];             // swmscsym
extern sopsym_t *symbol_burst[2];                // swbstsym
extern sopsym_t *symbol_plane[2][16];            // swplnsym
extern sopsym_t *symbol_plane_hit[2];            // swhitsym
extern sopsym_t *symbol_plane_win[4];            // swwinsym

extern sopsym_t symbol_pixel;

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 27/06/2002: move plane symbol headers here
// sdh 27/06/2002: add sopsym_t, sopsym_t sprite frame replacements
// sdh 21/10/2001: moved plane sprite constants into here from sw.h
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

