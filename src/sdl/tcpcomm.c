// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2003 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------------
//
// TCP/IP Communications
//
//---------------------------------------------------------------------------

#include "sw.h"
#include "swtitle.h"
#include "tcpcomm.h"
#include "timer.h"

#ifdef TCPIP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_net.h>

#define PORT 3847

static TCPsocket server_sock4, server_sock6;
static TCPsocket tcp_sock;

#endif   /* #ifdef TCPIP */

// connect to a host

void commconnect(char *host)
{
#ifdef TCPIP
	char *realhost;
	int port = PORT;
	IPaddress addr;
	int result;

	// sdh 17/11/2001: check for host:port, use a different
	// port from the default

	{
		char *p = strchr(host, ':');
		if (0 && p) {
			realhost = malloc(p-host + 2);
			strncpy(realhost, host, p-host);
			realhost[p-host] = '\0';
			port = atoi(p+1);
		} else {
			realhost = host;
		}
	}

	result = SDLNet_ResolveHost(SDLNET_ANY, &addr, realhost, port);
	
	if (realhost != host)
		free(realhost);

	if (result) {
		fprintf(stderr,
			"commconnect: cant resolve '%s': %s\n",
			host, SDLNet_GetError());
		exit(-1);
	}

	fprintf(stderr, "commconnect: trying to connect to host (%s), %s\n",
		addr.type == SDLNET_IPV6 ? "IPv6" :
		addr.type == SDLNET_IPV4 ? "IPv4" : "??",
		SDLNet_PresentIP(&addr));

	tcp_sock = SDLNet_TCP_Connect(&addr);

	if (!tcp_sock) {
		fprintf(stderr, 
			"commconnect: cant connect to host (%s)\n",
			SDLNet_GetError());
		exit(-1);
	}

#endif    /* #ifdef TCPIP */
}

// open a socket and listen until a connection is established

void commlisten()
{
#ifdef TCPIP
	SDLNet_SocketSet set;
	IPaddress *addr;

	server_sock6 = SDLNet_TCP_OpenServer(SDLNET_IPV6, PORT);
	server_sock4 = SDLNet_TCP_OpenServer(SDLNET_IPV4, PORT);
	
	if (!server_sock4 && !server_sock6) {
		fprintf(stderr, "commlisten: cant listen on any sockets\n");
		exit(-1);		
	}

	atexit(commterm);

	set = SDLNet_AllocSocketSet(2);

	if (server_sock6) {
		fprintf(stderr, "commlisten: listening on IPv6\n");
		SDLNet_TCP_AddSocket(set, server_sock6);
	}
	if (server_sock4) {
		fprintf(stderr, "commlisten: listening on IPv4\n");
		SDLNet_TCP_AddSocket(set, server_sock4);
	}

	// listen for connection

	for (;;) {
		if (ctlbreak()) {
			fprintf(stderr, 
				"commlisten: user aborted connect\n");
			exit(-1);
		}

		if (SDLNet_CheckSockets(set, 100) < 0) {
			fprintf(stderr,
				"commlisten: Error in SDLNet_CheckSockets: %s\n",
				SDLNet_GetError());
			exit(-1);
		}

		if (server_sock4 && SDLNet_SocketReady(server_sock4)) {
			tcp_sock = SDLNet_TCP_Accept(server_sock4);

			if (server_sock6) {
				SDLNet_TCP_Close(server_sock6);
				server_sock6 = NULL;
			}

			break;
		}

		if (server_sock6 && SDLNet_SocketReady(server_sock6)) {
			tcp_sock = SDLNet_TCP_Accept(server_sock6);

			if (server_sock4) {
				SDLNet_TCP_Close(server_sock4);
				server_sock4 = NULL;
			}

			break;
		}
	}

	if (!tcp_sock) {
		fprintf(stderr, "commlisten: error in accepting "
			"connection: %s\n", 
			SDLNet_GetError());

		exit(-1);
	}

	fprintf(stderr,
		"commlisten: accepted connection from %s\n",
		SDLNet_PresentIP(SDLNet_TCP_GetPeerAddress(tcp_sock)));
#endif   /* #ifdef TCPIP */
}

// read a byte from socket

int commin()
{
#ifdef TCPIP
	unsigned char c;
	int bytes;

	// lost connection

	if (!tcp_sock)
		return -1;

	// read

	bytes = SDLNet_TCP_Recv(tcp_sock, &c, 1);

	if (bytes < 0) {
		fprintf(stderr, "commin: %s reading from socket\n",
			SDLNet_GetError());
		exit(-1);
	} else if (bytes < 1) {
		return -1;
	}

	return c;
#else
	return 0;
#endif   /* #ifdef TCPIP */
}

// send a byte

void commout(unsigned char i)
{
#ifdef TCPIP
	if (tcp_sock < 0)
		return;

	if (!SDLNet_TCP_Send(tcp_sock, &i, 1)) {
		fprintf(stderr,
			"commout: %s writing to socket\n",
			SDLNet_GetError());
	}
#endif   /* #ifdef TCPIP */
}

// disconnect

void commterm()
{
#ifdef TCPIP
	if (tcp_sock) {
		SDLNet_TCP_Close(tcp_sock);
		tcp_sock = NULL;
	}
	if (server_sock4) {
		SDLNet_TCP_Close(server_sock4);
		server_sock4 = NULL;
	}
	if (server_sock6) {
		SDLNet_TCP_Close(server_sock4);
		server_sock4 = NULL;
	}
#endif      /* #ifdef TCPIP */
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/08/06 14:53:05  fraggle
// SDL_net implementation of tcpcomm.c
// This uses the new SDL_net API which is IPv6 compatible
//
//
//---------------------------------------------------------------------------
