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

#include "sw.h"

#define BOMBBYTES       16              /*  Bytes in a bomb symbol          */
#define BOMBANGS        8               /*  Number of bomb angles           */
#define TARGBYTES       64              /*  Bytes in a target symbol        */
#define TARGORIENTS     4               /*  Number of target types          */
#define EXPLSYMS        8               /*  Number of explosion symbols     */
#define EXPBYTES        16              /*  Bytes in an explosion symbol    */
#define FLCKSYMS        2               /*  Number of flock symbols         */
#define FLKBYTES        64              /*  Bytes in a flock symbol         */
#define BIRDSYMS        2               /*  Number of bird symbols          */
#define BRDBYTES        2               /*  Bytes in a bird symbol          */
#define OXSYMS          2               /*  Number of ox symbols            */
#define OXBYTES         64              /*  Bytes in an ox symbol           */
#define GHSTBYTES       16              /*  Bytes in a ghost symbol         */
#define SHOTBYTES       64              /*  Bytes in a shot window symbol   */
#define SPLTBYTES       256             /*  Bytes in a splatted bird symbol */
#define MISCBYTES       16              /*  Bytes in a missile symbol       */
#define MISCANGS        16              /*  Number of missile angles        */
#define BRSTBYTES       16              /*  Bytes in a starburst symbol     */
#define BRSTSYMS        2               /*  Number of starburst symbols     */

extern char    swbmbsym[BOMBANGS][BOMBBYTES];
extern char    swtrgsym[TARGORIENTS][TARGBYTES];
extern char    swhtrsym[TARGBYTES] ;
extern char    swexpsym[EXPLSYMS][EXPBYTES];
extern char    swflksym[FLCKSYMS][FLKBYTES];
extern char    swbrdsym[BIRDSYMS][BRDBYTES];
extern char    swoxsym[OXSYMS][OXBYTES];
extern char    swghtsym[GHSTBYTES];
extern char    swshtsym[SHOTBYTES];
extern char    swsplsym[SPLTBYTES];
extern char    swmscsym[MISCANGS][MISCBYTES];
extern char    swbstsym[BRSTSYMS][BRSTBYTES];

#endif


//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: moved plane sprite constants into here from sw.h
// sdh 21/10/2001: added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------

