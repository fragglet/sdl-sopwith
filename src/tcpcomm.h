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
// TCP/IP Communications
//

#ifndef __TCPCOMM_H__
#define __TCPCOMM_H__

extern void commconnect(char *host);
extern void commlisten(void);

extern int commin(void);
extern void commout(unsigned char c);

extern void commterm(void);

#endif /* __TCPCOMM_H__ */
