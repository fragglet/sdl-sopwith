// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001 Simon Howard
//
// All rights reserved except as specified in the file license.txt.
// Distribution of this file without the license.txt file accompanying
// is prohibited.
//
//---------------------------------------------------------------------------

#ifndef __BMBLIB_H__
#define __BMBLIB_H__

#include "sw.h"

extern int _systype;
extern int Args_CheckArg(int argc, char **argv, char arg);
extern int strindex(char *str,int c);
extern int inportb(unsigned port);
extern int outportb(unsigned port,int data);
extern void setmem(void *dest,unsigned count,int c);
extern void intson();
extern void intsoff();

#endif


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:02:45  fraggle
// Initial revision
//
//
// sdh 16/11/2001: movmem, setmem removed
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 19/10/2001: added header
//
//---------------------------------------------------------------------------
