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

#ifndef __STD_H__
#define __STD_H__

#include <stdio.h>
#include <stdlib.h>

#define FOREVER  for (;;)

// sdh 21/10/2001: moved BOOL here from sw.h, made into enum
typedef enum {FALSE, TRUE} BOOL;
enum {NO, YES};

int strindex(char *str,int c);
void movblock(unsigned int srcoff,unsigned int srcseg,
              unsigned int destoff,unsigned int destseg,
              unsigned int count);

// sdh: these are standard c anyway: memcpy and memset
// remove at some point

void movmem(void *src,void *dest,unsigned count);
void setmem(void *dest,unsigned count,int c);

#endif

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: rearranged headers, added cvs tags
//
//---------------------------------------------------------------------------

