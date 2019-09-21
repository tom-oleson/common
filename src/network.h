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

#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "util.h"
#include "thread.h"
#include "queue.h"
#include "log.h"

#define CM_NET_OK 0
#define CM_NET_ERR -1

namespace cm_net {


int create_socket();
int server_socket(int host_port);
void close_socket(int fd);
int accept(int host_socket, std::string &info);
int connect(const std::string &host, int host_port, std::string &info);

void send(int socket, const std::string &msg);
void send(int socket, char *buf, size_t buf_size, const std::string &msg);

int recv(int socket, char *buf, size_t buf_size);
int recv_non_block(int socket, char *buf, size_t buf_size);
int read(int fd, char *buf, size_t sz);
int write(int fd, char *buf, size_t sz);


int enable_reuseaddr(int fd);
int set_non_block(int fd, bool non_block);
int set_keep_alive(int fd);
int set_keep_alive_interval(int fd, int interval);
int set_no_delay(int fd, int no_delay);
int set_send_buffer(int fd, int size);
int set_receive_buffer(int fd, int size);
int set_send_timeout(int fd, long long millis);
int set_receive_timeout(int fd, long long millis);
int read(int fd, char *buf, size_t sz);
int write(int fd, char *buf, size_t sz);


// event-driven I/O
#define MAX_EVENTS  10
int epoll_create();
int add_socket(int epollfd, int fd, uint32_t flags);
int modify_socket(int epollfd, int fd, uint32_t flags);
int delete_socket(int epollfd, int fd);


inline void err(const std::string &msg, int errnum) {
    char buf[128] = {'\0'};
    strerror_r(errnum, buf, sizeof(buf));
    cm_log::error(cm_util::format("%s: %s", msg.c_str(), buf));
}

#define CM_NET_RECEIVE(fn) void (*fn)(int socket, const char *buf, size_t sz)

#define CM_NET_ON_RECEIVE(fn) void (*fn)(int socket, const char *buf, size_t sz)

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

    int host_port;
    std::string info;

    CM_NET_RECEIVE(receive_fn) = nullptr;

    char rbuf[4096] = { '\0' };
    char sbuf[4096] = { '\0' };

    int epollfd;
    int listen_socket;
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, timeout = -1;    

    bool setup();
    void cleanup();
    bool process();

    int accept();
    int do_use(int fd);
    
public:
    server_thread(int port, CM_NET_RECEIVE(fn));
    ~server_thread();

};

class client_thread: public cm_thread::basic_thread  {

protected:

    //std::unique_ptr<connection_thread> connection;
    connection_thread *connection = nullptr;

    std::string host;
    int socket;
    int host_port;
    std::string info;

    CM_NET_RECEIVE(receive_fn) = nullptr;

    bool setup();
    void cleanup();
    bool process();

    int connect();

    void send(const std::string msg) {
        connection->send(msg);
    }   

    // std::unique_ptr<connection_thread>
    // create_connection_thread(int socket, const std::string info) {
    //     return std::make_unique<connection_thread>(socket, info, receive_fn);
    // }

    // connection_thread *
    // create_connection_thread(int socket, const std::string info) {
    //     return new cm_net::connection_thread(socket, info, receive_fn);
    // }

public:
    client_thread(const std::string host, int port, CM_NET_RECEIVE(fn));
    ~client_thread();

};



} // namespace cm_net

#endif
