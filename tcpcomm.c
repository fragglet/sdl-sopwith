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

#include "sw.h"
#include "tcpcomm.h"

#define PORT 3847

static int server_sock = -1;
static int tcp_sock = -1;

// connect to a host

void commconnect(char *host)
{
	struct hostent *hent;
	struct sockaddr_in in;

	// resolve name

	hent = gethostbyname(host);

	if (!hent) {
		fprintf(stderr,
			"commconnect: cant resolve '%s': %s\n",
			host, strerror(errno));
		exit(-1);
	}
	// create socket

	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcp_sock < -1) {
		fprintf(stderr,
			"commconnect: cant create socket: %s\n",
			strerror(errno));
		exit(-1);
	}
	// connect

	in.sin_family = AF_INET;
	in.sin_addr.s_addr = ((struct in_addr *) hent->h_addr)->s_addr;
	in.sin_port = htons(PORT);

	if (connect(tcp_sock, (struct sockaddr *) &in, sizeof(in)) < 0) {
		fprintf(stderr,
			"commconnect: cant connect to '%s': %s\n",
			host, strerror(errno));
		exit(-1);
	}
	// non blocking socket

	fcntl(tcp_sock, F_SETFL, O_NONBLOCK);

	fprintf(stderr, "commconnect: connected to '%s'!\n", host);
}

// open a socket and listen until a connection is established

void commlisten()
{
	struct sockaddr_in in;
	int in_size;

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

	// accept connection

	in_size = sizeof(in);

	tcp_sock = accept(server_sock, (struct sockaddr *) &in, &in_size);

	if (tcp_sock < 0) {
		fprintf(stderr,
			"commlisten: error accepting connection: %s\n",
			strerror(errno));
		exit(-1);
	}
	// non blocking socket

	fcntl(tcp_sock, F_SETFL, O_NONBLOCK);

	fprintf(stderr,
		"commlisten: accepted connection from %s\n",
		inet_ntoa(in.sin_addr));
}

// read a byte from socket

int commin()
{
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
}

// send a byte

void commout(unsigned char i)
{
	if (tcp_sock < 0)
		return;

	if (!write(tcp_sock, &i, 1)) {
		fprintf(stderr,
			"commout: %s writing to socket\n",
			strerror(errno));
	}
}

// disconnect

void commterm()
{
	if (tcp_sock > 0) {
		close(tcp_sock);
		tcp_sock = -1;
	}

	if (server_sock > 0) {
		close(server_sock);
		server_sock = -1;
	}
}

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
//
//---------------------------------------------------------------------------
