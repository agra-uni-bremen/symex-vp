/*
 * Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * gpio-client.cpp
 *
 *  Created on: 5 Nov 2018
 *      Author: dwd
 */

#include "gpio-client.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#define ENABLE_DEBUG (0)
#include "debug.h"

using namespace std;

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

GpioClient::GpioClient() : fd(-1) {}

GpioClient::~GpioClient() {
	if (fd >= 0) {
		close(fd);
	}
}

bool GpioClient::update() {
	Request req;
	memset(&req, 0, sizeof(Request));
	req.op = GET_BANK;
	if (write(fd, &req, sizeof(Request)) != sizeof(Request)) {
		cerr << "Error in write " << fd << endl;
		return false;
	}
	if (read(fd, &state, sizeof(Reg)) != sizeof(Reg)) {
		cerr << "Error in read " << fd << endl;
		return false;
	}
	return true;
}

bool GpioClient::setBit(uint8_t pos, Tristate val) {
	Request req;
	memset(&req, 0, sizeof(Request));
	req.op = SET_BIT;
	req.setBit.pos = pos;
	req.setBit.val = val;

	if (write(fd, &req, sizeof(Request)) != sizeof(Request)) {
		cerr << "Error in write" << endl;
		return false;
	}
	return true;
}

bool GpioClient::setupConnection(const char *host, const char *port) {
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return false;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(fd);
			//perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		//fprintf(stderr, "client: failed to connect\n");
		freeaddrinfo(servinfo);
		return false;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	DEBUG("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);  // all done with this structure

	return true;
}
