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
#include <memory>
#include <algorithm>

#include "util.h"
#include "thread.h"
#include "log.h"

#define CM_NET_OK 0
#define CM_NET_ERR -1

namespace cm_net {

int enable_reuseaddr(int fd);
int create_socket();
int server_socket(int host_port);
void close_socket(int fd);
int accept(int host_socket, std::string &info);
int connect(const std::string &host, int host_port, std::string &info);
void send(int socket, char *buf, size_t buf_size, const std::string &msg);

inline void err(const std::string &msg, int errnum) {
    char buf[128] = {'\0'};
    strerror_r(errnum, buf, sizeof(buf));
    cm_log::error(cm_util::format("%s: %s", msg.c_str(), buf));
}

#define CM_NET_RECEIVE(fn) void (*fn)(const char *buf, size_t sz)

class connection_thread: public cm_thread::basic_thread {

    int socket;
    std::string info;

    CM_NET_RECEIVE(receive_fn) = nullptr;

    char rbuf[4096] = { '\0' };
    char sbuf[4096] = { '\0' };

    bool setup();
    void cleanup();
    bool process();

public:
    connection_thread(int socket, const std::string info, CM_NET_RECEIVE(fn));
    ~connection_thread();

    void send(const std::string &msg);
    void receive(const char *buf, size_t sz);

};

class server_thread: public cm_thread::basic_thread  {

protected:

    std::vector<std::unique_ptr<connection_thread>> connections;

    int host_port;
    int host_socket;
    std::string info;

    CM_NET_RECEIVE(receive_fn) = nullptr;

    bool setup();
    void cleanup();
    bool process();

    int accept();

    std::unique_ptr<connection_thread>
    create_connection_thread(int socket, const std::string info) {
        return std::make_unique<connection_thread>(socket, info, receive_fn);
    }

public:
    server_thread(int port, CM_NET_RECEIVE(fn));
    ~server_thread();

    void service_connection(int socket, const std::string info);

};


} // namespace cm_net


#endif
