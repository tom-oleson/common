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

#ifndef __NETWORK_H
#define __NETWORK_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include <vector>
#include <string>

#include "util.h"
#include "thread.h"
#include "log.h"

#define CM_NET_OK 0
#define CM_NET_ERR -1

namespace cm_net {

int create_socket();
int server_socket(int host_port);
void close_socket(int fd);
int accept(int host_socket, std::string &info);
int connect(const std::string &host, int host_port);

inline void err(const std::string &msg, int errnum) {
    char buf[128] = {'\0'};
    strerror_r(errnum, buf, sizeof(buf));
    cm_log::error(cm_util::format("%s: %s", msg.c_str(), buf));
}


class server_thread: cm_thread::basic_thread  {

protected:

    int host_port;
    int host_socket;
    std::string info;

    bool setup();
    void cleanup();
    bool process();

    int accept();

public:
    server_thread(int port);
    ~server_thread();

    void service_connection(int socket, const std::string info);

};

class connection_thread: cm_thread::basic_thread {

    int socket;
    std::string info;

    char rbuf[4096] = { '\0' };
    char sbuf[4096] = { '\0' };

    bool setup();
    void cleanup();
    bool process();

public:
    connection_thread(int socket, const std::string info);
    ~connection_thread();

};


} // namespace cm_net


#endif
