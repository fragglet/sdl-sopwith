// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 2001 Simon Howard
//
// This file is dual-licensed under version 2 of the GNU General Public
// License as published by the Free Software Foundation, and the Sopwith
// License as published by David L. Clark. See the files GPL and license.txt
// respectively for more information.
//
//--------------------------------------------------------------------------
//
// Configuration code
//
// Save game settings to a configuration file
//
//-------------------------------------------------------------------------

#ifndef __SWCONF_H__
#define __SWCONF_H__

typedef struct
{
	char *name;
	enum {
		CONF_BOOL,
		CONF_INT
	} type;
	union {
		void *v;
		BOOL *b;
		int *i;
	} value;
	char *description;
} confoption_t;

extern confoption_t confoptions[];
extern int num_confoptions;

extern void swloadconf();
extern void swsaveconf();
extern void setconfig();          // config menu

#endif

//-------------------------------------------------------------------------
//
// $Log: $
//
// sdh 10/11/2001: make confoptions available globally for gtk code
//
//-------------------------------------------------------------------------
