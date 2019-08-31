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

#include "sockets.h"


cm_socket::server_socket::server_socket(int port): host_port(port) {
    // start processing thread
    start();
}

cm_socket::server_socket::~server_socket() {
    // stop processing thread
    stop();

    // if there are clients, shut them down
}


bool cm_socket::server_socket::setup() {

    // create host socket
    
    host_socket = socket(AF_INET, SOCK_STREAM, 0 /*protocol*/);
    if(-1 == host_socket) {
        cm_log::error("failed to create host socket");
        return false;
    }

    // bind socket to IP/port

    server_hint.sin_family = AF_INET;
    server_hint.sin_port = htons(host_port);
    server_hint.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(host_socket, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_log::error("failed to bind to IP/port");
        return false;
    }

    // mark socket for listening

   if(-1 == listen(host_socket, SOMAXCONN)) {
        cm_log::error("listen call failed on host socket");
        close(host_socket);
        host_socket = -1;
        return false;
   } 

}

void cm_socket::server_socket::cleanup() {

    if(host_socket != -1) close(host_socket);
}

int cm_socket::server_socket::accept() {

    // wait for and accept a client connection

    socklen_t client_sz = sizeof(client_hint);
    return ::accept(host_socket, (sockaddr *) &client_hint, &client_sz);
}


bool cm_socket::server_socket::process() {

    int socket = accept();
    while(-1 == (socket = ::accept())) {
        if(errno != EINTR) return false;
        /* blocking accept interrupted, to loop for retry */
    }

    memset(host, 0, sizeof(host));
    memset(serv, 0, sizeof(serv));

    if( 0 == getnameinfo( (sockaddr *) &client_hint, sizeof(client_hint),
        host, sizeof(host), serv, sizeof(serv), 0 /*flags*/) ) {
        cm_log::info(cm_util::format("connection on %s:%s", host, serv));
    } 
    else {
        // no name info available, use info from client connection...
        if(NULL != inet_ntop(AF_INET, &client_hint, host, sizeof(host) )) {
            cm_log::info(cm_util::format("connected on %s:%s", host, ntohs( client_hint.sin_port)) );
        }
    } 
    
    client_connect(socket);
    

    return true;
} 


