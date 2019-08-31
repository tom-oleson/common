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
        cm_net::err("failed to create socket", errno);
        return CM_NET_ERR;
    }

    // bind socket to IP/port

    sockaddr_in server_hint;
    bzero(&server_hint, sizeof(server_hint));

    server_hint.sin_family = AF_INET;
    server_hint.sin_port = htons(host_port);
    server_hint.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(host_socket, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_net::err("error on bind to IP/port", errno);
        return CM_NET_ERR;
    }

    // mark socket for listening

   if(-1 == listen(host_socket, SOMAXCONN)) {
        cm_net::close_socket(host_socket);
        cm_net::err("error on listen", errno);
        return CM_NET_ERR;
   } 

   return host_socket;
}

void cm_net::close_socket(int fd) {

    if(fd != -1) close(fd);
}

int cm_net::accept(int host_socket, std::string &info) { 

    int fd = -1;

    sockaddr_in client_hint;
    socklen_t client_sz = sizeof(client_hint);
    bzero(&client_hint, sizeof(client_hint));

    char host[NI_MAXHOST] = { '\0' };
    char serv[NI_MAXSERV] = { '\0' };
    char info_buf[NI_MAXHOST + NI_MAXSERV + 1] = { '\0' };

    while(-1 == (fd = ::accept(host_socket, (sockaddr *) &client_hint, &client_sz))) {
        if(errno != EINTR) {
            cm_net::err("error on accept", errno);
            return CM_NET_ERR;
        }
        /* blocking accept interrupted, loop for retry */
    }

    if( 0 == getnameinfo( (sockaddr *) &client_hint, sizeof(client_hint),
        host, sizeof(host), serv, sizeof(serv), 0 /*flags*/) ) {
        snprintf(info_buf, sizeof(info_buf), "%s:%s", host, serv);
    }
    else if( NULL != inet_ntop(AF_INET, &client_hint, host, sizeof(host) ))  {
        // no name info available, use info from client connection...
        snprintf(info_buf, sizeof(info_buf), "%s:%d", host, ntohs(client_hint.sin_port));
    }

    info.assign(info_buf);

    return fd;
}

int cm_net::connect(const std::string &host, int host_port) {

    // create socket

    int fd = cm_net::create_socket();
    if(-1 == fd) {
        cm_net::err("failed to create socket", errno);
        return CM_NET_ERR;
    }

    // get host info

    hostent *host_ent;
    if(NULL == (host_ent = gethostbyname(host.c_str())) ) {
        cm_net:err(cm_util::format("gethostbyname failed: %s", host.c_str()), errno);
        return CM_NET_ERR;
    }

    sockaddr_in server_hint;
    bzero(&server_hint, sizeof(server_hint));
    server_hint.sin_family = AF_INET;
    server_hint.sin_port = htons(host_port);
    server_hint.sin_addr = *((in_addr *) host_ent->h_addr);
    bzero(&(server_hint.sin_zero), 8);

    if(-1 == ::connect(fd, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_net::err("failed to connect to IP/port", errno);
        cm_net::close_socket(fd);
        return CM_NET_ERR;
    }

    return fd;
}
