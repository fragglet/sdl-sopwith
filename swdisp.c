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
//        swdispc  -      Display all players and objects
//
//---------------------------------------------------------------------------

#include "sw.h"
#include "swdisp.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swutil.h"

static void plnsound(OBJECTS *obp)
{
	register OBJECTS *ob = obp;

	if (ob->ob_firing)
		sound(S_SHOT, 0, ob);
	else
		switch (ob->ob_state) {
		case FALLING:
			if (ob->ob_dy >= 0)
				sound(S_HIT, 0, ob);
			else
				sound(S_FALLING, ob->ob_y, ob);
			break;

		case FLYING:
			sound(S_PLANE, -ob->ob_speed, ob);
			break;

		case STALLED:
		case WOUNDED:
		case WOUNDSTALL:
			sound(S_HIT, 0, ob);
			break;

		default:
			break;
		}

}



void dispplyr(OBJECTS * ob)
{
	if (shothole)
		dispwindshot();
	if (splatbird)
		dispsplatbird();
        if (splatox)
                dispoxsplat();

	plnsound(ob);
}






void dispbomb(OBJECTS * obp)
{
	register OBJECTS *ob = obp;

	if (ob->ob_dy <= 0)
		sound(S_BOMB, -ob->ob_y, ob);
}





void dispmiss(OBJECTS * obp)
{
}





void dispburst(OBJECTS * obp)
{
}





void dispexpl(OBJECTS * obp)
{
	register OBJECTS *ob = obp;

	if (ob->ob_orient)
		sound(S_EXPLOSION, ob->ob_hitcount, ob);
}





void dispcomp(OBJECTS * ob)
{
	plnsound(ob);
}




void dispmult(OBJECTS * ob)
{
	plnsound(ob);
}




void disptarg(OBJECTS * ob)
{
	if (ob->ob_firing)
		sound(S_SHOT, 0, ob);
}




void dispflck(OBJECTS * ob)
{
}




void dispbird(OBJECTS * ob)
{
}



void dispwobj(OBJECTS * obp)
{
	register OBJECTS *ob;
	register OLDWDISP *ow;
//      int               ox, oy;
	int oldplot;

	ob = obp;
	ow = &wdisp[ob->ob_index];

	if (ow->ow_xorplot)
		swpntsym(ow->ow_x, ow->ow_y, ow->ow_xorplot - 1);

	if (ob->ob_state >= FINISHED)
		ow->ow_xorplot = 0;
	else {
		ow->ow_x = SCR_CENTR
			   + (ob->ob_x + ob->ob_symwdt / 2) / WRLD_RSX;
		ow->ow_y = (ob->ob_y - ob->ob_symhgt / 2) / WRLD_RSY;

		oldplot = swpntcol(ow->ow_x,
				   ow->ow_y,
				   ob->ob_owner->ob_clr);

		if (oldplot == 0 || (oldplot & 0x0003) == 3) {
			ow->ow_xorplot = oldplot + 1;
			return;
		}
		swpntsym(ow->ow_x, ow->ow_y, oldplot);
		ow->ow_xorplot = 0;
	}
}

#define SEED_START 74917777

static unsigned long seed = SEED_START;

unsigned long randsd()
{
	seed *= countmove;
	seed += 7491;

	if (!seed)
		seed = SEED_START;

	return 0;
}


void dispwindshot()
{
	OBJECTS ob;

	// sdh 28/10/2001: option to disable hud splats

	if (!conf_hudsplats)
		return;

	ob.ob_type = DUMMYTYPE;
	ob.ob_symhgt = ob.ob_symwdt = 16;
	ob.ob_clr = 0;
	ob.ob_newsym = swshtsym;

	do {
		randsd();
		swputsym((unsigned) (seed % (SCR_WDTH - 16)),
			 (unsigned) (seed % (SCR_HGHT - 50)) + 50, &ob);
	} while (--shothole);
}



void dispsplatbird()
{
	OBJECTS ob;

	// sdh 28/10/2001: option to disable hud splats

	if (!conf_hudsplats)
		return;

	ob.ob_type = DUMMYTYPE;
	ob.ob_symhgt = ob.ob_symwdt = 32;
	ob.ob_clr = 2;
	ob.ob_newsym = swsplsym;

	do {
		randsd();
		swputsym((unsigned) (seed % (SCR_WDTH - 32)),
			 (unsigned) (seed % (SCR_HGHT - 60)) + 60, &ob);
	} while (--splatbird);
}




void dispoxsplat()
{
	register OBJECTS *ob;
	register int i;

	// sdh 28/10/2001: option to disable hud splats

	if (!conf_hudsplats)
		return;

	colorscreen(2);

	swsetblk(0, SCR_SEGM,
		 ((SCR_HGHT - SCR_MNSH - 2) >> 1) * SCR_LINW, 0xAA);
	swsetblk(SCR_ROFF, SCR_SEGM,
		 ((SCR_HGHT - SCR_MNSH - 3) >> 1) * SCR_LINW, 0xAA);
	splatox = 0;
	oxsplatted = 1;

	ob = nobjects;
	for (i = 0; i < MAX_OBJS; ++i, ob++)
		ob->ob_drwflg = ob->ob_delflg = 0;

	dispinit = TRUE;
}


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 28/10/2001: option to disable hud splats
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed extern definitions: these are now in headers
//                 shuffled some functions around to shut up compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-04-05        Missile and starburst support
// 87-03-13        Splatted bird symbol.
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 84-06-12        PCjr Speed-up
// 84-02-21        Development
//
//---------------------------------------------------------------------------

