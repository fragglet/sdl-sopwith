//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
// TCP/IP Communications
//

#include "swasynio.h"
#include "swtitle.h"
#include "tcpcomm.h"
#include "timer.h"
#include "video.h"

#include "config.h"

#if defined(HAVE_NETINET_IN_H) || defined(HAVE_WINSOCK_H)
#define TCPIP
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_NETINET_IN_H
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#define closesocket close
#endif /* HAVE_NETINET_IN_H */

#ifdef HAVE_WINSOCK_H
#include <winsock.h>
typedef int socklen_t;
#endif /* HAVE_WINSOCK_H */

static int server_sock = -1;
static int tcp_sock = -1;

static void comminit(void)
{
	static int initialized = 0;

	if (initialized) {
		return;
	}

#ifdef HAVE_WINSOCK_H
	{
		WORD winsock11 = MAKEWORD(1, 1);
		WSADATA wsaData;

		if (WSAStartup(winsock11, &wsaData) != 0) {
			error_exit("Error initializing winsock.");
		}
	}
#endif
	initialized = 1;
}

// poll_socket returns 1 if there appears to be new data ready on the given
// socket. It uses select() because this works everywhere, while every OS
// seems to have their own unique way of setting a non-blocking socket.
static int poll_socket(int s)
{
	fd_set in_fds, except_fds;
	struct timeval timeout = { 0, 1 };  // 1us = ~immediate

	FD_ZERO(&in_fds);
	FD_SET(s, &in_fds);
	FD_ZERO(&except_fds);
	FD_SET(s, &except_fds);
	if (select(s + 1, &in_fds, NULL, &except_fds, &timeout) < 0) {
		// error may mean error on socket, so take a look.
		return 1;
	}
	return FD_ISSET(s, &in_fds) || FD_ISSET(s, &except_fds);
}

// connect to a host

void commconnect(char *host)
{
#ifdef TCPIP
	struct hostent *hent;
	struct sockaddr_in in;
	char *realhost;
	int port = asynport;

	comminit();

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

	if (realhost != host) {
		free(realhost);
	}

	if (!hent) {
		// TODO: Handle this more softly than exiting the whole
		// program; let the user type the address again?
		error_exit("commconnect: cant resolve '%s': %s", host,
		           strerror(errno));
	}
	// create socket

	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcp_sock < 0) {
		error_exit("commconnect: can't create socket: %s",
		           strerror(errno));
	}

	// connect

	in.sin_family = AF_INET;
	in.sin_addr.s_addr = ((struct in_addr *) hent->h_addr)->s_addr;
	in.sin_port = htons(port);

	if (connect(tcp_sock, (struct sockaddr *) &in, sizeof(in)) < 0) {
		error_exit("commconnect: cant connect to '%s': %s",
		           host, strerror(errno));
	}
	fprintf(stderr, "commconnect: connected to '%s'!\n", host);
#endif    /* #ifdef TCPIP */
}

// open a socket and listen until a connection is established

void commlisten(void)
{
#ifdef TCPIP
	struct sockaddr_in in;
	socklen_t in_size;

	comminit();

	// create socket

	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (server_sock < 0) {
		error_exit("commconnect: can't create socket: %s",
		           strerror(errno));
	}
	// bind to port

	memset(&in, 0, sizeof(in));
	in.sin_family = AF_INET;
	in.sin_addr.s_addr = INADDR_ANY;
	in.sin_port = htons(asynport);

	if (bind(server_sock, (struct sockaddr *) &in, sizeof(in)) < 0) {
		error_exit("commlisten: cant bind to port %d: %s",
		           asynport, strerror(errno));
	}

	atexit(commterm);

	// listen for connections

	if (listen(server_sock, 1)) {
		error_exit("commlisten: cant listen on port %i: %s",
		           asynport, strerror(errno));
	}

	fprintf(stderr,
		"commlisten: listening for connection on port %i\n", asynport);

	// listen for connection

	for (;;) {
		if (ctlbreak()) {
			error_exit("commlisten: user aborted connect");
		}

		if (!poll_socket(server_sock)) {
			Timer_Sleep(50);
			continue;
		}

		in_size = sizeof(in);
		tcp_sock = accept(server_sock, (struct sockaddr *) &in, &in_size);

		if (tcp_sock >= 0) {
			break;
		}

		error_exit("commlisten: error accepting connection: %s",
		           strerror(errno));
	}

	fprintf(stderr, "commlisten: accepted connection from %s\n",
	        inet_ntoa(in.sin_addr));
#endif   /* #ifdef TCPIP */
}

// read a byte from socket

int commin(void)
{
#ifdef TCPIP
	unsigned char c;
	int bytes;

	// lost connection

	if (tcp_sock < 0) {
		return -1;
	}

	if (!poll_socket(tcp_sock)) {
		return -1;
	}

	// read
	bytes = recv(tcp_sock, (char *) &c, 1, 0);

	if (bytes < 1) {
		error_exit("commin: %s reading from socket",
		           strerror(errno));
		return 0;
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
	if (tcp_sock < 0) {
		return;
	}

	if (!send(tcp_sock, (char *) &i, 1, 0)) {
		error_exit("commout: %s writing to socket",
		           strerror(errno));
	}
#endif   /* #ifdef TCPIP */
}

// disconnect

void commterm(void)
{
#ifdef TCPIP
	if (tcp_sock > 0) {
		closesocket(tcp_sock);
		tcp_sock = -1;
	}

	if (server_sock > 0) {
		closesocket(server_sock);
		server_sock = -1;
	}
#endif      /* #ifdef TCPIP */
}

bool isNetworkGame(void)
{
#ifdef TCPIP
	if (tcp_sock > 0) {
		return true;
	}
#endif   /* #ifdef TCPIP */

	return false;
}