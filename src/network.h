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

#include "ssl.h"
#include "store.h"

#define CM_NET_OK 1
#define CM_NET_EOF 0
#define CM_NET_ERR -1
#define CM_NET_AGAIN -2
#define CM_NET_WANT_WRITE -3

namespace cm_net {


int create_socket(int domain);
int create_socket();
int server_socket_inet6(int host_port);
int server_socket(int host_port, int domain);
int server_socket(int host_port);
void close_socket(int fd);

int accept_inet6(int host_socket, std::string &info);
int accept(int host_socket, std::string &info);
int gethostbyname(const std::string &host, hostent **host_ent);

int connect_inet6(const std::string &host, int host_port, std::string &info);
int connect(const std::string &host, int host_port, std::string &info);

void send(int socket, const std::string &msg);
void send(int socket, char *buf, size_t buf_size, const std::string &msg);

int recv(int socket, char *buf, size_t buf_size);
int recv_non_block(int socket, char *buf, size_t buf_size);
int read(int fd, char *buf, size_t sz);
int write(int fd, char *buf, size_t sz);

int resolve_host(const std::string &host, std::string &info, int flags);
int resolve_host(const std::string &host, std::string &info);
int resolve_host_ip(const std::string &host, std::string &info);
int enable_reuseaddr(int fd);
int set_IPv6_only(int fd);
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
#define MAX_EVENTS  64
int epoll_create();
int add_socket(int epollfd, int fd, uint32_t flags);
int modify_socket(int epollfd, int fd, uint32_t flags);
int delete_socket(int epollfd, int fd);


inline void err(const std::string &msg, int errnum) {
    CM_LOG_ERROR {
        cm_log::error(cm_util::format("%s: (errno %d) %s", msg.c_str(), errnum, strerror(errnum)));
    }
}

inline void err(const std::string &msg) {
    CM_LOG_ERROR {
        cm_log::error(cm_util::format("%s", msg.c_str()));
    }
}

#define cm_net_receive(fn) void (*fn)(int socket, const char *buf, size_t sz)

class single_thread_server: public cm_thread::basic_thread  {

protected:

    int host_port;
    std::string info;

    cm_net_receive(receive_fn) = nullptr;

    char rbuf[4096] = { '\0' };

    int epollfd;
    int listen_socket;
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, timeout = -1;    

    bool setup();
    void cleanup();
    bool process();

    int accept();
    int service_input_event(int fd);
    
public:
    single_thread_server(int port, cm_net_receive(fn));
    ~single_thread_server();
};


struct rx_thread: public cm_thread::basic_thread  {

    int socket;
    bool connected = false;
    int epollfd = -1;

    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, timeout = -1;    

    cm_net_receive(receive_fn) = nullptr;
    rx_thread *rx = nullptr;

    //char rbuf[4096] = { '\0' };
   
    bool setup();
    void cleanup();
    bool process();

    int service_input_event(int fd);

    rx_thread(int s, cm_net_receive(fn));
    ~rx_thread();
};


class client_thread: public cm_thread::basic_thread  {

protected:

    std::string host;
    std::string info;

    int socket;
    int host_port;

    bool connected = false;
    int epollfd = -1;

    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, timeout = 100; // ms timeout    

    cm_net_receive(receive_fn) = nullptr;
    //rx_thread *rx = nullptr;

    bool setup();
    void cleanup();
    bool process();

    int connect();
    int service_input_event(int fd);
    void send(const std::string msg);

public:
    client_thread(const std::string host, int port, cm_net_receive(fn));
    ~client_thread();

    int get_socket() { return socket; }
    bool is_connected() { return connected; }
};

////////////////////// SSL single_thread_server ////////////////////////

//#define cm_net_ssl_receive(fn) void (*fn)(SSL *ssl, const char *buf, size_t sz)

class single_thread_server_ssl: public cm_thread::basic_thread  {

protected:

    cm_store::info_store<int, cm_ssl::ssl_bio *> ssl_store;

    int host_port;
    std::string info;

    SSL_CTX *ctx;
    cm_ssl::ssl_bio *bio;

    ssl_receive_cb(receive_fn) = nullptr;

    char rbuf[4096] = { '\0' };

    int epollfd;
    int listen_socket;
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, timeout = 100;    // ms timeout

    bool setup();
    void cleanup();
    bool process();

    int accept();
    int service_input_event(int fd);
    void remove_fd(int fd);
    
public:
    single_thread_server_ssl(int port, ssl_receive_cb(fn));
    ~single_thread_server_ssl();

    int ssl_write(const char *buf, size_t sz) {
        return bio->do_ssl_write(buf, sz);
    }

};

/////////////////////////// pool server ///////////////////////////////

struct input_event {
    int fd = -1;        // response socket
    std::string msg;    // input data
    bool eof = false;       // signals EOF (disonnected)
    bool connect = false;   // signals connect event

    input_event() {}
    ~input_event() {}

    input_event(int fd_): fd(fd_), eof(true) {}
    input_event(int fd_, std::string msg_): fd(fd_), msg(msg_) {}
    input_event(const input_event &r): fd(r.fd), msg(r.msg), eof(r.eof),
        connect(r.connect) {}
    input_event &operator = (const input_event &r) {
        fd = r.fd;
        msg = r.msg;
        eof = r.eof;
        connect = r.connect;
        return *this;
    }
};

class pool_server: public cm_thread::basic_thread  {

protected:

    int host_port;
    std::string info;

    cm_thread::pool *pool = nullptr;
    cm_task_function(receive_fn) = nullptr;
    cm_task_dealloc(dealloc) = nullptr;

    int epollfd;
    int listen_socket;
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, timeout = 10;    // ms timeout

    bool setup();
    void cleanup();
    bool process();

    int accept();
    int service_input_event(int fd);
    int service_connect_event(int fd, const std::string info);
    
public:
    pool_server(int port, cm_thread::pool *pool,
         cm_task_function(fn), cm_task_dealloc(dealloc_));
    ~pool_server();
};

////////////////////// SSL client_thread //////////////////////////

class client_thread_ssl: public cm_thread::basic_thread  {

protected:

    cm_store::info_store<int, cm_ssl::ssl_bio *> ssl_store;

    std::string host;
    std::string info;

    SSL_CTX *ctx = nullptr;
    X509 *cert = nullptr;
    cm_ssl::ssl_bio *bio;

    int socket;
    int host_port;

    ssl_receive_cb(receive_fn) = nullptr;
    //char rbuf[4096] = { '\0' };

    bool connected = false;
    int epollfd = -1;

    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, timeout = 100;  // ms timeout

    bool setup();
    void cleanup();
    bool process();

    int connect();
    int service_input_event(int fd);
    void remove_fd(int fd);

public:
    client_thread_ssl(const std::string host, int port, ssl_receive_cb(fn));
    ~client_thread_ssl();

    int ssl_write(const char *buf, size_t sz) {
        return bio->do_ssl_write(buf, sz);
    }

    bool is_connected() { return connected; }

};

} // namespace cm_net

#endif
