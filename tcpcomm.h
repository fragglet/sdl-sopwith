// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//
// TCP/IP Communications
//
//---------------------------------------------------------------------------

#ifndef __TCPCOMM_H__
#define __TCPCOMM_H__

extern void commconnect(char *host);
extern void commlisten();

extern int commin();
extern void commout(unsigned char c);

extern void commterm();

#endif /* __TCPCOMM_H__ */

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
//
//---------------------------------------------------------------------------

