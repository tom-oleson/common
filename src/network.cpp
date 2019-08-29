/*
 * Copyright (c) 2019, Tom Oleson <tom dot oleson at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * The names of its contributors may NOT be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "network.h"


int cm_net::create_socket() {

    int fd = socket(AF_INET, SOCK_STREAM, 0 /*protocol*/);
    return fd;
}

int cm_net::server_socket(int host_port) {

    // create host socket
    
    int host_socket = cm_net::create_socket();
    if(-1 == host_socket) {
        cm_log::error("failed to create socket");
        return CM_NET_ERR;
    }

    // bind socket to IP/port

    sockaddr_in server_hint;
    server_hint.sin_family = AF_INET;
    server_hint.sin_port = htons(host_port);
    server_hint.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(host_socket, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_log::error("failed to bind to IP/port");
        return CM_NET_ERR;
    }

    // mark socket for listening

   if(-1 == listen(host_socket, SOMAXCONN)) {
        cm_log::error("listen call failed");
        close(host_socket);
        return CM_NET_ERR;
   } 

    return host_socket;

}

void cm_net::close_socket(int fd) {

    if(fd != -1) close(fd);
}

int cm_net::accept_connection(int host_socket, sockaddr *client_hint, socklen_t *client_sz) {

    int fd = -1;

    while(-1 == (fd = ::accept(host_socket, client_hint, client_sz))) {
        if(errno != EINTR) return CM_NET_ERR;
        /* blocking accept interrupted, loop for retry */
    }

    return fd;
}

