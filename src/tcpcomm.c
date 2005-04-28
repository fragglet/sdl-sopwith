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
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define PORT 3847

static int server_sock = -1;
static int tcp_sock = -1;

#endif   /* #ifdef TCPIP */

// connect to a host

void commconnect(char *host)
{
#ifdef TCPIP
	struct hostent *hent;
	struct sockaddr_in in;
	char *realhost;
	int port = PORT;

	// sdh 17/11/2001: check for host:port, use a different
	// port from the default

	{
		char *p = strchr(host, ':');
		if (p) {
			realhost = malloc(p-host + 2);
			strncpy(realhost, host, p-host);
			realhost[p-host] = '\0';
			port = atoi(p+1);
		} else {
			realhost = host;
		}
	}
       
	// resolve name

	hent = gethostbyname(realhost);

	if (realhost != host)
		free(realhost);

	if (!hent) {
		fprintf(stderr,
			"commconnect: cant resolve '%s': %s\n",
			host, strerror(errno));
		exit(-1);
	}
	// create socket

	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcp_sock < 0) {
		fprintf(stderr,
			"commconnect: cant create socket: %s\n",
			strerror(errno));
		exit(-1);
	}

	// connect

	in.sin_family = AF_INET;
	in.sin_addr.s_addr = ((struct in_addr *) hent->h_addr)->s_addr;
	in.sin_port = htons(port);

	if (connect(tcp_sock, (struct sockaddr *) &in, sizeof(in)) < 0) {
		fprintf(stderr,
			"commconnect: cant connect to '%s': %s\n",
			host, strerror(errno));
		exit(-1);
	}
	// non blocking socket

	fcntl(tcp_sock, F_SETFL, O_NONBLOCK);

	fprintf(stderr, "commconnect: connected to '%s'!\n", host);
#endif    /* #ifdef TCPIP */
}

// open a socket and listen until a connection is established

void commlisten()
{
#ifdef TCPIP
	struct sockaddr_in in;
	size_t in_size;

	// create socket

	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (server_sock < 0) {
		fprintf(stderr,
			"commlisten: could not create socket: %s\n",
			strerror(errno));
		exit(-1);
	}
	// bind to port

	memset(&in, 0, sizeof(in));
	in.sin_family = AF_INET;
	in.sin_addr.s_addr = INADDR_ANY;
	in.sin_port = htons(PORT);

	if (bind(server_sock, (struct sockaddr *) &in, sizeof(in)) < 0) {
		fprintf(stderr,
			"commlisten: cant bind to port %i: %s\n",
			PORT, strerror(errno));
		exit(-1);
	}

	atexit(commterm);

	// listen for connections

	if (listen(server_sock, 1)) {
		fprintf(stderr,
			"commlisten: cant listen on port %i: %s\n",
			PORT, strerror(errno));
		exit(-1);
	}

	fprintf(stderr,
		"commlisten: listening for connection on port %i\n", PORT);

		
	fcntl(server_sock, F_SETFL, O_NONBLOCK);
	
	// listen for connection

	for (;;) {
		if (ctlbreak()) {
			fprintf(stderr, 
				"commlisten: user aborted connect\n");
			exit(-1);
		}

		in_size = sizeof(in);
		
		tcp_sock = accept(server_sock, (struct sockaddr *) &in, &in_size);

		if (tcp_sock >= 0) {
			break;
		}

		if (errno == EWOULDBLOCK) {
			Timer_Sleep(100);
		} else {
			fprintf(stderr,
				"commlisten: error accepting connection: %s\n",
				strerror(errno));
			exit(-1);
		}
	}

	// non blocking socket
	
	fcntl(tcp_sock, F_SETFL, O_NONBLOCK);

	fprintf(stderr,
		"commlisten: accepted connection from %s\n",
		inet_ntoa(in.sin_addr));
#endif   /* #ifdef TCPIP */
}

// read a byte from socket

int commin()
{
#ifdef TCPIP
	unsigned char c;
	int bytes;

	// lost connection

	if (tcp_sock < 0)
		return -1;

	// read

	bytes = read(tcp_sock, &c, 1);

	if (bytes < 1) {
		if (errno != EWOULDBLOCK) {
			fprintf(stderr,
				"commin: %s reading from socket\n",
				strerror(errno));
			close(tcp_sock);
			tcp_sock = -1;
		}
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

	if (!write(tcp_sock, &i, 1)) {
		fprintf(stderr,
			"commout: %s writing to socket\n",
			strerror(errno));
	}
#endif   /* #ifdef TCPIP */
}

// disconnect

void commterm()
{
#ifdef TCPIP
	if (tcp_sock > 0) {
		close(tcp_sock);
		tcp_sock = -1;
	}

	if (server_sock > 0) {
		close(server_sock);
		server_sock = -1;
	}
#endif      /* #ifdef TCPIP */
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.3  2005/04/28 14:52:55  fraggle
// Fix compilation under gcc 4.0
//
// Revision 1.2  2003/04/05 22:55:11  fraggle
// Remove the FOREVER macro and some unused stuff from std.h
//
// Revision 1.1.1.1  2003/02/14 19:03:23  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 17/11/2001: ability to specify a different port from default
// sdh 16/11/2001: TCPIP #define to disable TCP/IP support
// sdh 21/10/2001: added cvs tags
//
//---------------------------------------------------------------------------
