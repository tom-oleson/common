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

// Resolve host name to its IP address
int cm_net::resolve_host(const std::string &host, std::string &info, int flags) {

    struct addrinfo hints, *res;
    int rv;

    bzero(&hints, sizeof(hints));
    char ip_buf[NI_MAXHOST] = { '\0' };

    if (flags != 0) hints.ai_flags = flags;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  

    if ((rv = getaddrinfo(host.c_str(), NULL, &hints, &res)) != 0) {
        cm_net::err(cm_util::format("getaddrinfo failed: %s", gai_strerror(rv)));
        return CM_NET_ERR;
    }
    if (res->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)res->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ip_buf, sizeof(ip_buf));
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)res->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ip_buf, sizeof(ip_buf));
    }

    freeaddrinfo(res);
    info.assign(info);
    
    return CM_NET_OK;
}

int cm_net::connect_inet6(const std::string &host, int host_port, std::string &info) {

    struct addrinfo hints, *res;
    int fd = -1, rv;
    char port_str[6] = { '\0' };

    snprintf(port_str, sizeof(port_str), "%d", host_port); 

    bzero(&hints, sizeof(hints));
    char ip_buf[NI_MAXHOST] = { '\0' };

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  

    if ((rv = getaddrinfo(host.c_str(), port_str, &hints, &res)) != 0) {
        cm_net::err(cm_util::format("getaddrinfo failed: %s", gai_strerror(rv)));
        return CM_NET_ERR;
    }

    if (res->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)res->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ip_buf, sizeof(ip_buf));
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)res->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ip_buf, sizeof(ip_buf));
    }

    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        cm_net::err("create socket failed", errno);
        return CM_NET_ERR;
    }

    // we must do this on *all* sockets we create or we will get
    // a bind error when we try to use the same local address again
    // (e.g., after a restart)
    if(CM_NET_ERR == cm_net::enable_reuseaddr(fd)) {
        close_socket(fd);
        return CM_NET_ERR;
    }

    if(-1 == ::connect(fd,res->ai_addr,res->ai_addrlen)) {
        cm_net::err("connect_inet6 failed", errno);
        cm_net::close_socket(fd);
        return CM_NET_ERR;      
    }

    // get info for this client connection.
    // we are after the port the server selected for our connect()
    sockaddr_in6 client_hint;
    char _host[NI_MAXHOST] = { '\0' };
    char _serv[NI_MAXSERV] = { '\0' };
    char info_buf[NI_MAXHOST + NI_MAXSERV + 1] = { '\0' };
    socklen_t client_len = sizeof(client_hint);

    if( 0 == getsockname(fd, (sockaddr *) &client_hint, &client_len)) { 
        if( 0 == getnameinfo( (sockaddr *) &client_hint, sizeof(client_hint),
            _host, sizeof(_host), _serv, sizeof(_serv), 0 /*flags*/) ) {
            snprintf(info_buf, sizeof(info_buf), "%s:%s", _host, _serv);
        }
        else if( NULL != inet_ntop(AF_INET6, &client_hint, _host, sizeof(_host) ))  {
            // no name info available, use info from client connection...
            snprintf(info_buf, sizeof(info_buf), "%s:%d", _host, ntohs(client_hint.sin6_port));
        }
    }
    info.assign(info_buf);

    freeaddrinfo(res);

    return fd;
}

int cm_net::resolve_host(const std::string &host, std::string &info) {
    return cm_net::resolve_host(host, info,  0 /*flags*/);
}

int cm_net::resolve_host_ip(const std::string &host, std::string &info) {
    return cm_net::resolve_host(host, info, AI_NUMERICHOST /*flags*/);
}

int cm_net::enable_reuseaddr(int fd) {
    int enable = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
        cm_net::err("setsockopt: SO_REUSEADDR", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::create_socket(int domain) {

    int fd = socket(domain, SOCK_STREAM, 0 /*protocol*/);
     if(-1 == fd) {
        cm_net::err("create socket failed", errno);
        return CM_NET_ERR;
    }   

    // we must do this on *all* sockets we create or we will get
    // a bind error when we try to use the same local address again
    // (e.g., after a restart)
    if(CM_NET_ERR == cm_net::enable_reuseaddr(fd)) {
        close_socket(fd);
        return CM_NET_ERR;
    }

    return fd;
}

int cm_net::create_socket() {
    return cm_net::create_socket(AF_INET);  // IPv4
}

int cm_net::server_socket_inet6(int host_port) {

    // create host socket
    int host_socket = cm_net::create_socket(AF_INET6);
    if(CM_NET_ERR == host_socket) {
        return CM_NET_ERR;
    }

    // bind socket to IP/port
    sockaddr_in6 server_hint;
    bzero(&server_hint, sizeof(server_hint));
    
    server_hint.sin6_family = AF_INET6;
    server_hint.sin6_port = htons(host_port);
    // allows (by default) both IPv4 and IPv6 clients to connect
    server_hint.sin6_addr = in6addr_any;

    if(-1 == bind(host_socket, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_net::err("bind failed", errno);
        return CM_NET_ERR;
    }
    
    // mark socket for listening
    if(-1 == listen(host_socket, SOMAXCONN)) {
        cm_net::close_socket(host_socket);
        cm_net::err("listen failed", errno);
        return CM_NET_ERR;
    } 
   
    cm_log::info(cm_util::format("listening on port %d", host_port)); 

    return host_socket;
}

int cm_net::server_socket(int host_port, int domain) {

    // create host socket
    int host_socket = cm_net::create_socket(domain);
    if(CM_NET_ERR == host_socket) {
        return CM_NET_ERR;
    }

    // bind socket to IP/port
    sockaddr_in server_hint;
    bzero(&server_hint, sizeof(server_hint));

    //server_hint.sin_family = AF_INET;
    server_hint.sin_family = domain;
    server_hint.sin_port = htons(host_port);
    server_hint.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(host_socket, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_net::err("bind failed", errno);
        return CM_NET_ERR;
    }
    
    // mark socket for listening
    if(-1 == listen(host_socket, SOMAXCONN)) {
        cm_net::close_socket(host_socket);
        cm_net::err("listen failed", errno);
        return CM_NET_ERR;
    } 
   
    cm_log::info(cm_util::format("listening on port %d", host_port)); 

    return host_socket;
}

int cm_net::server_socket(int host_port) {
    return cm_net::server_socket(host_port, AF_INET);   // IPv4
}

int cm_net::shutdown(int fd, int how) {
    return shutdown(fd, how);
}

void cm_net::close_socket(int fd) {

    if(fd != -1) {
        //cm_net::shutdown(fd, SHUT_RDWR);
        close(fd);
    }
}



int cm_net::accept_inet6(int host_socket, std::string &info) { 

    int fd = -1;

    sockaddr_in6 client_hint;
    socklen_t client_sz = sizeof(client_hint);
    bzero(&client_hint, sizeof(client_hint));

    while(-1 == (fd = ::accept(host_socket, (sockaddr *) &client_hint, &client_sz))) {
        if(errno != EINTR) {
            cm_net::err("accept failed", errno);
            return CM_NET_ERR;
        }
        /* accept interrupted, loop for retry */
    }

    char host[NI_MAXHOST] = { '\0' };
    char serv[NI_MAXSERV] = { '\0' };
    char info_buf[NI_MAXHOST + NI_MAXSERV + 1] = { '\0' };

    if( 0 == getnameinfo( (sockaddr *) &client_hint, sizeof(client_hint),
        host, sizeof(host), serv, sizeof(serv), 0 /*flags*/) ) {
        snprintf(info_buf, sizeof(info_buf), "%s:%s", host, serv);
    }
    else if( NULL != inet_ntop(AF_INET6, &client_hint, host, sizeof(host) ))  {
        // no name info available, use info from client connection...
        snprintf(info_buf, sizeof(info_buf), "%s:%d", host, ntohs(client_hint.sin6_port));
    }
    info.assign(info_buf);

    return fd;
}

int cm_net::accept(int host_socket, std::string &info) { 

    int fd = -1;

    sockaddr_in client_hint;
    socklen_t client_sz = sizeof(client_hint);
    bzero(&client_hint, sizeof(client_hint));

    while(-1 == (fd = ::accept(host_socket, (sockaddr *) &client_hint, &client_sz))) {
        if(errno != EINTR) {
            cm_net::err("accept failed", errno);
            return CM_NET_ERR;
        }
        /* accept interrupted, loop for retry */
    }

    char host[NI_MAXHOST] = { '\0' };
    char serv[NI_MAXSERV] = { '\0' };
    char info_buf[NI_MAXHOST + NI_MAXSERV + 1] = { '\0' };

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

int cm_net::gethostbyname(const std::string &host, hostent **host_ent) {
    hostent *p;
    if(NULL == (p = ::gethostbyname(host.c_str())) ) {
        cm_log::error(cm_util::format("gethostbyname failed: %s", host.c_str()));
        return CM_NET_ERR;
    }
    if(nullptr != host_ent) *host_ent = p;
    return CM_NET_OK;
}

int cm_net::connect(const std::string &host, int host_port, std::string &info) {

    // create socket
    int fd = cm_net::create_socket();
    if(CM_NET_ERR == fd) {
        return CM_NET_ERR;
    }

    // get host info
    hostent *host_ent;
    if(NULL == (host_ent = ::gethostbyname(host.c_str())) ) {
        cm_log::error(cm_util::format("gethostbyname failed: %s", host.c_str()));
        return CM_NET_ERR;
    }

    sockaddr_in server_hint;
    bzero(&server_hint, sizeof(server_hint));
    server_hint.sin_family = AF_INET;
    server_hint.sin_port = htons(host_port);
    server_hint.sin_addr = *((in_addr *) host_ent->h_addr);
    bzero(&(server_hint.sin_zero), 8);

    if(-1 == ::connect(fd, (sockaddr *) &server_hint, sizeof(server_hint))) {
        cm_net::err("connect failed", errno);
        cm_net::close_socket(fd);
        return CM_NET_ERR;
    }

    // get info for client connection. we are after the port the server selected
    // for our connect()

    sockaddr_in client_hint;
    char _host[NI_MAXHOST] = { '\0' };
    char _serv[NI_MAXSERV] = { '\0' };
    char info_buf[NI_MAXHOST + NI_MAXSERV + 1] = { '\0' };
    socklen_t client_len = sizeof(client_hint);

    if( 0 == getsockname(fd, (sockaddr *) &client_hint, &client_len)) { 
        if(0 == getnameinfo( (sockaddr *) &client_hint, sizeof(client_hint),
            _host, sizeof(_host), _serv, sizeof(_serv), 0 /*flags*/) ) {
            snprintf(info_buf, sizeof(info_buf), "%s:%s", _host, _serv);
        }
        else if( NULL != inet_ntop(AF_INET, &client_hint, _host, sizeof(_host) ))  {
            // no name info available, use info from client connection...
            snprintf(info_buf, sizeof(info_buf), "%s:%d", _host, ntohs(client_hint.sin_port));
        }
    }

    info.assign(info_buf);

    return fd;
}

void cm_net::send(int socket, const std::string &msg) {
    
    ::send(socket, msg.c_str(), msg.size(), 0 /*flags*/);
}

void cm_net::send(int socket, char *buf, size_t buf_size, const std::string &msg) {

    bzero(buf, buf_size);
    size_t sz = std::min(msg.size(),buf_size);
    memcpy(buf, msg.c_str(), sz);
    ::send(socket, buf, sz, 0 /*flags*/);
}

int cm_net::recv(int socket, char *buf, size_t buf_size) {

    bzero(buf, buf_size);
    int num_bytes = ::recv(socket, buf, buf_size, 0 /*flags*/);
    if(-1 == num_bytes) {
        cm_net::err(cm_util::format("recv error: socket=%d: ", socket), errno);
        return CM_NET_ERR;
    }
    return num_bytes;
}

int cm_net::recv_non_block(int socket, char *buf, size_t buf_size) {

    bzero(buf, buf_size);
    int num_bytes = ::recv(socket, buf, buf_size, MSG_DONTWAIT /*flags*/);
    if(-1 == num_bytes) {
        if(errno != EAGAIN && errno != EWOULDBLOCK) {
            cm_net::err(cm_util::format("recv error: socket=%d: ", socket), errno);
        }
        return CM_NET_ERR;
    }
    return num_bytes;
}

int cm_net::write(int fd, const char *buf, size_t sz) {

    // Write until sz bytes have been written (or error/EOF).
    ssize_t num_bytes, total_bytes = 0;
    while(total_bytes != sz) {
        num_bytes = ::write(fd, buf, sz - total_bytes);
        if (num_bytes == 0) return total_bytes;     // EOF
        if (num_bytes == -1) return -1;             // error
        total_bytes += num_bytes;
        buf += num_bytes;
    }

    return total_bytes;
}

int cm_net::set_IPv6_only(int fd) {
    int opt_ipv6_only = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_ipv6_only, sizeof(int)) == -1) {
        cm_net::err("setsockopt: IPV6_V6ONLY", errno);
        close(fd);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::set_non_block(int fd, bool non_block) {

    // get current flags for fd
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags) {
        cm_net::err("fcntl(F_GETFL)", errno);
        return CM_NET_ERR;
    }

    // merge in mask bits 
    if (non_block) {
        flags |= O_NONBLOCK;
    }
    else {
        flags &= ~O_NONBLOCK;
    }

    // set flags for fd
    if (-1 == fcntl(fd, F_SETFL, flags)) {
        cm_net::err("fcntl(F_SETFL,O_NONBLOCK)", errno);
        return CM_NET_ERR;
    }

    return CM_NET_OK;
}

// If set, disable the Nagle algorithm. This means that segments are always sent as
// soon as possible, even if there is only a small amount of data. When not set, data
// is buffered until there is a sufficient amount to send out, thereby avoiding the
// frequent sending of small packets, which results in poor utilization of the network. 

int cm_net::set_no_delay(int fd, int no_delay) {
    int opt_nodelay = no_delay;
    if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt_nodelay, sizeof(int))) {
        cm_net::err("setsockopt TCP_NODELAY", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::set_keep_alive(int fd) {

    // Enable sending of keep-alive messages on connection-oriented
    // sockets. Expects an integer boolean flag.
    int opt_keepalive = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt_keepalive, sizeof(int))) {
        cm_net::err("setsockopt: SO_KEEPALIVE", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::set_keep_alive_interval(int fd, int interval) {

    if (CM_NET_ERR == cm_net::set_keep_alive(fd)) {
        return CM_NET_ERR;
    }

#define __LINUX_KEEP_ALIVE__
#ifdef __LINUX_KEEP_ALIVE__

    //The time (in seconds) the connection needs to remain idle before TCP
    //starts sending keepalive probes, if the socket option SO_KEEPALIVE has
    //been set on this socket. This option should not be used in code intended
    //to be portable. 
    int opt_keepidle = interval;
    if (0 > setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &opt_keepidle, sizeof(int))) {
        cm_net::err("setsockopt: TCP_KEEPIDLE", errno);
        return CM_NET_ERR;
    }

    //The time (in seconds) between individual keepalive probes. This option
    // should not be used in code intended to be portable. 
    int opt_keepintvl = interval/3;
    if (opt_keepintvl == 0) opt_keepintvl = 1;
    if (0 > setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &opt_keepintvl, sizeof(int))) {
        cm_net::err("setsockopt: TCP_KEEPINTVL", errno);
        return CM_NET_ERR;
    }

    //The maximum number of keepalive probes TCP should send before dropping the
    //connection. This option should not be used in code intended to be portable.
    int opt_keepcnt = 3;
    if (0 > setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &opt_keepcnt, sizeof(int)) < 0) {
        cm_net::err("setsockopt: TCP_KEEPCNT", errno);
        return CM_NET_ERR;
    }
#else
    (void) interval;  // black hole
#endif

    return CM_NET_OK;
}

int cm_net::set_send_buffer(int fd, int size) {

    // Sets the maximum socket send buffer in bytes. The kernel doubles 
    // this value (to allow space for bookkeeping overhead) when
    // it is set using setsockopt(2), and this doubled value is returned
    // by getsockopt(2). The minimum (doubled) value for this option is 2048. 

    int opt_buf_size = size;
    if(opt_buf_size < 2048) opt_buf_size = 2048;
    if(-1 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &opt_buf_size, sizeof(int))) {
        cm_net::err("setsockopt SO_SNDBUF", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::set_receive_buffer(int fd, int size) {

    // Sets the maximum socket receive buffer in bytes. The kernel doubles 
    // this value (to allow space for bookkeeping overhead) when
    // it is set using setsockopt(2), and this doubled value is returned
    // by getsockopt(2). The minimum (doubled) value for this option is 256. 

    int opt_buf_size = size;
    if(opt_buf_size < 256) opt_buf_size = 256;
    if(-1 == setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &opt_buf_size, sizeof(int))) {
        cm_net::err("setsockopt SO_RCVBUF", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::set_send_timeout(int fd, long long millis) {

    // Set the sending timeouts until reporting an error.
    // If output function blocks for this period of time, and data has
    // been sent, the return value of that function will be the amount
    // of data transferred; if no data has been transferred and the timeout
    // has been reached then -1 is returned with errno set. If the timeout
    // is set to zero (the default) then the operation will never
    // timeout.

    struct timeval tv;
    tv.tv_sec = millis / 1000;
    tv.tv_usec = (millis % 1000) * 1000;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(timeval))) {
        cm_net::err("setsockopt SO_SNDTIMEO", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::set_receive_timeout(int fd, long long millis) {

    // Set the receiving timeouts until reporting an error.
    // If input function blocks for this period of time, and data has
    // been received, the return value of that function will be the amount
    // of data transferred; if no data has been transferred and the timeout
    // has been reached then -1 is returned with errno set. If the timeout
    // is set to zero (the default) then the operation will never
    // timeout.

    struct timeval tv;
    tv.tv_sec = millis / 1000;
    tv.tv_usec = (millis % 1000) * 1000;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval))) {
        cm_net::err("setsockopt SO_RCVTIMEO", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::read(int fd, char *buf, size_t sz) {

    // Read until sz bytes have been read (or error/EOF).
    ssize_t num_bytes, total_bytes = 0;
    while(total_bytes != sz) {
        num_bytes = ::read(fd, buf, sz - total_bytes);

        if (num_bytes == 0) return total_bytes;     // EOF/disconnect

        // no more data... return what we have
        if (num_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK) && total_bytes > 0)
            return total_bytes; 

        if (num_bytes == -1) return -1;             // error

        total_bytes += num_bytes;
        buf += num_bytes;
    }

    return total_bytes;
}

//////////////////// single_thread_server  //////////////////////////////

cm_net::single_thread_server::single_thread_server(int port,
    cm_net_receive(fn)): host_port(port), receive_fn(fn) {
    // start processing thread
    start();
}

cm_net::single_thread_server::~single_thread_server() {
    // stop processing thread
    stop();
}

bool cm_net::single_thread_server::setup() {

    epollfd = epoll_create();
    if(CM_NET_ERR == epollfd) {
        return false;
    }

    //listen_socket = cm_net::server_socket(host_port);
    // allow both IPv4 and IPv6 clients to connect
    listen_socket = cm_net::server_socket_inet6(host_port);
    if(-1 == listen_socket) {
        return false;
    }

    if(CM_NET_ERR == cm_net::add_socket(epollfd, listen_socket, EPOLLIN)) {
        cm_net::close_socket(listen_socket);
        return false;
    }

    return true;
}

void cm_net::single_thread_server::cleanup() {

    cm_net::close_socket(listen_socket);
}

int cm_net::single_thread_server::accept() {
    
    //int fd = cm_net::accept(listen_socket, info);
    int fd = cm_net::accept_inet6(listen_socket, info);
    if(CM_NET_ERR != fd) {
    
        if(CM_NET_ERR == cm_net::set_non_block(fd, true)) {
            cm_net::close_socket(fd);
            return CM_NET_ERR;
        }
    }

    return fd;
}

bool cm_net::single_thread_server::process() {

    // fetch fds that are ready for I/O...
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
    if(-1 == nfds) {
        cm_net::err("epoll_wait", errno);
        return false;
    }

    // process the ready fds
    for(int n = 0; n < nfds; ++n) {

        int fd = events[n].data.fd;

        if(fd == listen_socket) {
           conn_sock = accept();
            if(CM_NET_ERR != conn_sock) {
                cm_net::add_socket(epollfd, conn_sock, EPOLLIN | EPOLLET);
            }
        }
        else {
            // handle IO event...
            int result = service_input_event(fd);

            if(CM_NET_ERR == result || CM_NET_EOF == result) {
                delete_socket(epollfd, fd);
                cm_net::close_socket(fd);
                cm_log::info(cm_util::format("%d: closed connection", fd));
            }

            // handle peer shutdown
            if(events[n].events & EPOLLRDHUP) {
                // remove socket from interest list...
                if(CM_NET_OK == result) {
                    delete_socket(epollfd, fd);
                    cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection (EPOLLRDHUP).", fd));
                }
                cm_log::info(cm_util::format("%d: peer shutdown", fd));
            }           
        }
    }

    return true;
}

int cm_net::single_thread_server::service_input_event(int fd) {

    while(1) {
        
        int num_bytes = cm_net::read(fd, rbuf, sizeof(rbuf));

        if(num_bytes > 0) {
            // give data to callback function...
            receive_fn(fd, rbuf, num_bytes);
            return CM_NET_OK;
        }
        
        if(num_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // back to caller for the next epoll_wait()
            return CM_NET_OK;
        }

        if(num_bytes == 0) {
            return CM_NET_EOF;
        }

        if(num_bytes == -1) {
            cm_net::err("read", errno);
            return CM_NET_ERR;
        }
    }
}

/////////////////////// pool server //////////////////////////////

cm_net::pool_server::pool_server(int port, cm_thread::pool *pool_,
    cm_task_function(fn), cm_task_dealloc(dealloc_)): host_port(port),
     pool(pool_), receive_fn(fn), dealloc(dealloc_) {

    // start processing thread
    start();
}

cm_net::pool_server::~pool_server() {
    // stop processing thread
    stop();
}

bool cm_net::pool_server::setup() {

    epollfd = epoll_create();
    if(CM_NET_ERR == epollfd) {
        return false;
    }

    //listen_socket = cm_net::server_socket(host_port);
    // allow both IPv4 and IPv6 clients to connect
    listen_socket = cm_net::server_socket_inet6(host_port);
    if(-1 == listen_socket) {
        return false;
    }

    if(CM_NET_ERR == cm_net::add_socket(epollfd, listen_socket, EPOLLIN)) {
        cm_net::close_socket(listen_socket);
        return false;
    }

    return true;
}

void cm_net::pool_server::cleanup() {

    cm_net::close_socket(listen_socket);
}

int cm_net::pool_server::accept() {
    
    //int fd = cm_net::accept(listen_socket, info);
    int fd = cm_net::accept_inet6(listen_socket, info);
    if(CM_NET_ERR != fd) {
    
        if(CM_NET_ERR == cm_net::set_non_block(fd, true)) {
            cm_net::close_socket(fd);
            return CM_NET_ERR;
        }
    }

    return fd;
}

bool cm_net::pool_server::process() {

    // fetch fds that are ready for I/O...
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
    if(-1 == nfds) {
        cm_net::err("epoll_wait", errno);
        return false;
    }

    // process the ready fds
    for(int n = 0; n < nfds; ++n) {

        int fd = events[n].data.fd;

        if(fd == listen_socket) {
           conn_sock = accept();
            if(CM_NET_ERR != conn_sock) {
                cm_net::add_socket(epollfd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP);
                cm_log::info(cm_util::format("%d: connected: %s", conn_sock, info.c_str()));

                service_connect_event(conn_sock, info);
            }
        }
        else {
            // handle IO event...
            int result = service_input_event(fd);

            if(CM_NET_ERR == result || CM_NET_EOF == result) {
                delete_socket(epollfd, fd);
                cm_net::close_socket(fd);
                cm_log::info(cm_util::format("%d: closed connection", fd));

                service_disconnect_event(fd, info);
            }

            // handle peer shutdown
            if(events[n].events & EPOLLRDHUP) {
                // remove socket from interest list...
                if(CM_NET_OK == result) {
                    delete_socket(epollfd, fd);
                    cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection (EPOLLRDHUP).", fd));

                    service_disconnect_event(fd, info);
                }
                cm_log::info(cm_util::format("%d: peer shutdown", fd));
            }   
        }
    }

    return true;
}

int cm_net::pool_server::service_connect_event(int fd, const std::string info) {
    
    // give the response fd and singal connect to the thread pool
    input_event *event = new input_event(fd, info);
    event->connect = true;
    if(nullptr != event) {
        pool->add_task(receive_fn, event, dealloc);
    }
    else {
        cm_log::critical("pool_server: error: event allocation failed!");
        return CM_NET_ERR;
    }

    return CM_NET_OK;
}

int cm_net::pool_server::service_disconnect_event(int fd, const std::string info) {
    
    // EOF - client disconnected
    // give the response fd and singal EOF to the thread pool
    input_event *event = new input_event(fd, info);
    event->eof = true;
    if(nullptr != event) {
        pool->add_task(receive_fn, event, dealloc);
    }
    else {
        cm_log::critical("pool_server: error: event allocation failed!");
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::pool_server::service_input_event(int fd) {

    char rbuf[4096] = {'\0'};

    //while(1) {
        
        int num_bytes = cm_net::read(fd, rbuf, sizeof(rbuf));
        if(num_bytes > 0) {
            // give the response fd and data to the thread pool
            input_event *event = new input_event(fd, std::string(rbuf, num_bytes));
            if(nullptr != event) {
                pool->add_task(receive_fn, event, dealloc);
            }
            else {
                cm_log::critical("pool_server: error: event allocation failed!");
                return CM_NET_ERR;
            }
            return CM_NET_OK;
        }
        
        if(num_bytes == -1 && errno == EAGAIN) {
            // back to caller for the next epoll_wait()
            return CM_NET_OK;
        }

        if(num_bytes == 0) {
            return CM_NET_EOF;
        }

        if(num_bytes == -1) {
            cm_net::err("read", errno);
            return CM_NET_ERR;
        }

	return CM_NET_OK;
    //}
}

/////////////////////// rx_thread ///////////////////////////////

cm_net::rx_thread::rx_thread(int s, cm_net_receive(fn)):
     socket(s), receive_fn(fn) {
    // start processing thread
    start();
}

cm_net::rx_thread::~rx_thread() {
    // stop processing thread
    stop();
}

bool cm_net::rx_thread::setup() {

    connected = false;

    if(-1 == epollfd) {
        epollfd = epoll_create();
        if(CM_NET_ERR == epollfd) {
            return false;
        }
    }
    
    if(CM_NET_ERR == cm_net::add_socket(epollfd, socket, EPOLLIN | EPOLLRDHUP)) {
        cm_net::close_socket(socket);
        return false;
    }

    connected = true;

    return true;
}

void cm_net::rx_thread::cleanup() {

    cm_net::close_socket(socket);
    connected = false;
}

bool cm_net::rx_thread::process() {

    // fetch fds that are ready for I/O...
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
    if(-1 == nfds) {
        cm_net::err("epoll_wait", errno);
        return false;
    }

    // process the ready fds
    for(int n = 0; n < nfds; ++n) {
        int fd = events[n].data.fd;

        if(fd == socket) {

            // handle IO event...
            int result = service_input_event(fd);
            if(CM_NET_ERR == result) {
                cm_net::err("service_input_event", errno);
            }

            if(CM_NET_ERR == result || CM_NET_EOF == result) {
                connected = false;
                delete_socket(epollfd, fd);
                cm_net::close_socket(fd);
                cm_log::info(cm_util::format("%d: closed connection", fd));
            }

            // handle peer shutdown
            if(events[n].events & EPOLLRDHUP) {
                connected = false;
                // remove socket from interest list...
                if(CM_NET_OK == result) {
                    delete_socket(epollfd, fd);
                    cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection (EPOLLRDHUP)", fd));
                }
                cm_log::info(cm_util::format("%d: peer shutdown", fd));
            }           

            if(CM_NET_EOF == result) {
                return false;
            }

        }
    }

    return true;
}

int cm_net::rx_thread::service_input_event(int fd) {

    char rbuf[4096] = { '\0' };

    while(1) {
        
        int num_bytes = cm_net::read(fd, rbuf, sizeof(rbuf));

        if(num_bytes > 0) {
            // give data to callback function...
            receive_fn(fd, rbuf, num_bytes);
            return CM_NET_OK;
        }
        
        if(num_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))  {
            // back to caller for the next epoll_wait()
            return CM_NET_OK;
        }

        if(num_bytes == 0) {
            // EOF - client disconnected
            return CM_NET_EOF;
        }

        if(num_bytes == -1) {
            cm_net::err("read", errno);
            return CM_NET_ERR;
        }
    }
}

//////////////////// client_thread //////////////////////////////

cm_net::client_thread::client_thread(const std::string _host, int port, cm_net_receive(fn)):
     host(_host), host_port(port), receive_fn(fn) {
    // start processing thread
    start();
}

cm_net::client_thread::~client_thread() {
    // stop processing thread
    stop();
}

int cm_net::client_thread::connect() {

    //int fd = cm_net::connect(host, host_port, info);
    int fd = cm_net::connect_inet6(host, host_port, info);
    if(CM_NET_ERR != fd) {

        if(CM_NET_ERR == set_non_block(fd, true)) {
            cm_net::close_socket(fd);
            return CM_NET_ERR;
        }
    }

    return fd;
}

bool cm_net::client_thread::setup() {
    
    connected = false;

    socket = cm_net::client_thread::connect();
    if(CM_NET_ERR == socket) {
        return false;
    }

    if(-1 == epollfd) {
        epollfd = epoll_create();
        if(CM_NET_ERR == epollfd) {
            return false;
        }
    }
    
    if(CM_NET_ERR == cm_net::add_socket(epollfd, socket, EPOLLIN | EPOLLRDHUP)) {
        cm_net::close_socket(socket);
        return false;
    }

    connected = true;    
    
    cm_log::info(cm_util::format("client: connected: %s", info.size() > 0 ? info.c_str():
        "host?:serv?"));

    return true;
}

void cm_net::client_thread::cleanup() {
    if(socket != CM_NET_ERR) {
        cm_net::close_socket(socket);
        socket = -1;
    }
    connected = false;
}

bool cm_net::client_thread::process() {

    // fetch fds that are ready for I/O...
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
    if(-1 == nfds) {
        cm_net::err("epoll_wait", errno);
        return false;
    }

    // process the ready fds
    for(int n = 0; n < nfds; ++n) {
        int fd = events[n].data.fd;

        if(fd == socket) {

            // handle IO event...
            int result = service_input_event(fd);
            if(CM_NET_ERR == result) {
                cm_net::err("service_input_event", errno);
            }

            if(CM_NET_ERR == result || CM_NET_EOF == result) {
                connected = false;
                delete_socket(epollfd, fd);
                cm_net::close_socket(fd);
                cm_log::info(cm_util::format("%d: closed connection", fd));
            }

            // handle peer shutdown
            if(events[n].events & EPOLLRDHUP) {
                connected = false;
                // remove socket from interest list...
                if(CM_NET_OK == result) {
                    delete_socket(epollfd, fd);
                    cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection (EPOLLRDHUP)", fd));
                }
                cm_log::info(cm_util::format("%d: peer shutdown", fd));
            }           

            if(CM_NET_EOF == result) {
                return false;
            }

        }
    }

    return true; 
}

int cm_net::client_thread::service_input_event(int fd) {

    char rbuf[4096] = { '\0' };

    //while(1) {
        
        int num_bytes = cm_net::read(fd, rbuf, sizeof(rbuf));

        if(num_bytes > 0) {
            // give data to callback function...
            receive_fn(fd, rbuf, num_bytes);
            return CM_NET_OK;
        }
        
        if(num_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // back to caller for the next epoll_wait()
            return CM_NET_OK;
        }

        if(num_bytes == 0) {
            // EOF - client disconnected
            return CM_NET_EOF;
        }

        if(num_bytes == -1) {
            cm_net::err("read", errno);
            return CM_NET_ERR;
        }
    //}
        return CM_NET_OK;
}

void cm_net::client_thread::send(const std::string msg) {
    cm_net::send(socket, msg);
}

/////////////////////// event-driven I/O /////////////////////////////////

int cm_net::epoll_create() {
    int fd = epoll_create1(0);
    if(-1 == fd) {
        cm_net::err("epoll_create", errno);
        return CM_NET_ERR;
    }
    return fd;
}

int cm_net::add_socket(int epollfd, int fd, uint32_t flags) {
    struct epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        cm_net::err(cm_util::format("%d: epoll_ctl:ADD", fd), errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::modify_socket(int epollfd, int fd, uint32_t flags) {
    struct epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        cm_net::err(cm_util::format("%d: epoll_ctl:MOD", fd), errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::delete_socket(int epollfd, int fd) {
    struct epoll_event ev;
    ev.events = 0;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        cm_net::err(cm_util::format("%d: epoll_ctl:DEL", fd), errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

//////////////////// single_thread_server_ssl  //////////////////////////////

cm_net::single_thread_server_ssl::single_thread_server_ssl(int port,
    ssl_receive_cb(fn)): host_port(port), receive_fn(fn) {
    // start processing thread
    start();
}

cm_net::single_thread_server_ssl::~single_thread_server_ssl() {
    // stop processing thread
    stop();
}

bool cm_net::single_thread_server_ssl::setup() {

    cm_ssl::init_openssl();
    ctx = cm_ssl::ctx_create();
    cm_ssl::ctx_configure(ctx);

    epollfd = epoll_create();
    if(CM_NET_ERR == epollfd) {
        return false;
    }

    // allow both IPv4 and IPv6 clients to connect
    listen_socket = cm_net::server_socket_inet6(host_port);
    if(-1 == listen_socket) {
        return false;
    }

    if(CM_NET_ERR == cm_net::add_socket(epollfd, listen_socket, EPOLLIN | EPOLLET)) {
        cm_net::close_socket(listen_socket);
        return false;
    }

    return true;
}

void cm_net::single_thread_server_ssl::cleanup() {

    cm_net::close_socket(listen_socket);
    cm_ssl::ctx_free(ctx);
    cm_ssl::cleanup_openssl();
}

int cm_net::single_thread_server_ssl::accept() {
    
    //int fd = cm_net::accept(listen_socket, info);
    int fd = cm_net::accept_inet6(listen_socket, info);
    if(CM_NET_ERR != fd) {
    
        if(CM_NET_ERR == cm_net::set_non_block(fd, true)) {
            cm_net::close_socket(fd);
            return CM_NET_ERR;
        }
    }

    return fd;
}

void cm_net::single_thread_server_ssl::remove_fd(int fd) {

    cm_ssl::ssl_bio *p = ssl_store.get(fd, nullptr);
    if(nullptr != p) {
        ssl_store.remove(fd);
        delete p;
    }

    cm_net::delete_socket(epollfd, fd);
    cm_net::close_socket(fd);
}

bool cm_net::single_thread_server_ssl::process() {

    // fetch fds that are ready for I/O...
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
    if(-1 == nfds) {
        cm_net::err("epoll_wait", errno);
        return false;
    }

    // process the ready fds
    for(int n = 0; n < nfds; ++n) {

        int fd = events[n].data.fd;

        if(fd == listen_socket) {
           conn_sock = single_thread_server_ssl::accept();
            if(CM_NET_ERR != conn_sock) {

                SSL *ssl = cm_ssl::ssl_create(ctx);
                bio = new cm_ssl::ssl_bio(ssl, conn_sock, true /*is_server*/, receive_fn);
                if(nullptr == bio) {
                    // failed to create bio object
                    cm_ssl::ssl_free(ssl);
                    cm_net::close_socket(conn_sock);
                    cm_log::error("Failed to create new ssl_bio");
                    cm_log::info(cm_util::format("%d: closed connection", conn_sock));
                    continue;
                }
                
                ssl_store.set(conn_sock, bio);
                cm_net::add_socket(epollfd, conn_sock, EPOLLIN | EPOLLET);
                cm_ssl::ssl_accept(bio->ssl);

                cm_log::info(cm_util::format("%d: connected: %s (%s)",
                     conn_sock, info.c_str(), cm_ssl::ssl_get_version(ssl)));
            }
        }
        else {
            int result = CM_NET_ERR;

            bio = ssl_store.get(fd, nullptr);
            if(nullptr == bio) {
                cm_log::error("Failed to find bio for fd");
                continue;
            }

            if(events[n].events & EPOLLIN || events[n].events & EPOLLOUT) {
                result = service_input_event(fd);
                //if(CM_NET_ERR == result || CM_NET_EOF == result) {
                if(CM_NET_EOF == result) {
                    remove_fd(fd);
                    //cm_net::delete_socket(epollfd, fd);
                    //cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection", fd));
                }
            }

            // handle peer shutdown
            if(events[n].events & EPOLLRDHUP) {
                // remove socket from interest list...
                if(CM_NET_OK == result) {
                    remove_fd(fd);
                    //cm_net::delete_socket(epollfd, fd);
                    //cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection (EPOLLRDHUP).", fd));
                }
                cm_log::info(cm_util::format("%d: peer shutdown", fd));
            }           
        }
    }

    return true;
}

int cm_net::single_thread_server_ssl::service_input_event(int fd) {
    
    cm_log::trace("service_input_event");

    while(1) {
        
        int status = bio->service_io();
        if(status == CM_NET_AGAIN) {
            // back to caller for the next epoll_wait()
            return CM_NET_OK;
        }

        if(status == CM_NET_EOF) {
            // EOF - client disconnected
            return CM_NET_EOF;
        }

        if(status == CM_NET_ERR) {
            cm_net::err("service_io", errno);
            return CM_NET_ERR;
        }
    }
}

//////////////////// client_thread_ssl //////////////////////////////

cm_net::client_thread_ssl::client_thread_ssl(const std::string _host,
     int port, ssl_receive_cb(fn)): host(_host), host_port(port),
     receive_fn(fn) {
    // start processing thread
    start();
}

cm_net::client_thread_ssl::~client_thread_ssl() {
    // stop processing thread
    stop();
}

int cm_net::client_thread_ssl::connect() {

    //int fd = cm_net::connect(host, host_port, info);
    int fd = cm_net::connect_inet6(host, host_port, info);
    if(CM_NET_ERR != fd) {

        if(CM_NET_ERR == set_non_block(fd, true)) {
            cm_net::close_socket(fd);
            return CM_NET_ERR;
        }
    }

    return fd;
}

bool cm_net::client_thread_ssl::setup() {
    
    connected = false;

    cm_ssl::init_openssl();
    ctx = cm_ssl::ctx_create();
    cm_ssl::client_ctx_configure(ctx);

    if(-1 == epollfd) {
        epollfd = epoll_create();
        if(CM_NET_ERR == epollfd) {
            return false;
        }
    }

    socket = cm_net::client_thread_ssl::connect();
    if(CM_NET_ERR == socket) {
        return false;
    }
    
    SSL *ssl = cm_ssl::ssl_create(ctx);
    bio = new cm_ssl::ssl_bio(ssl, socket, false /*is_server*/, receive_fn);
    if(nullptr == bio) {
        // failed to create bio object
        cm_ssl::ssl_free(ssl);
        cm_net::close_socket(socket);
        cm_log::error("Failed to create new ssl_bio");
        cm_log::info(cm_util::format("%d: closed connection", socket));
        return false;
    }
    
    ssl_store.set(socket, bio);
    cm_net::add_socket(epollfd, socket, EPOLLIN | EPOLLET);
    
    connected = true;    

    cm_log::info(cm_util::format("connected to: %s (%s)",
     info.c_str(), cm_ssl::ssl_get_version(bio->ssl)));

    bio->do_handshake();     

    // request server certificate
    cert = bio->do_get_peer_certificate(); 

    return true;
}

void cm_net::client_thread_ssl::cleanup() {
    if(socket != CM_NET_ERR) {
        cm_net::close_socket(socket);
        socket = -1;
    }
    if(nullptr != cert) cm_ssl::ssl_X509_free(cert);
    cm_ssl::ctx_free(ctx);
    cm_ssl::cleanup_openssl();    
    connected = false;
}

void cm_net::client_thread_ssl::remove_fd(int fd) {

    //cm_ssl::ssl_free(ssl);
    cm_net::delete_socket(epollfd, fd);
    cm_net::close_socket(fd);
}

bool cm_net::client_thread_ssl::process() {

    // fetch fds that are ready for I/O...
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
    if(-1 == nfds) {
        cm_net::err("epoll_wait", errno);
        return false;
    }

    // process the ready fds
    for(int n = 0; n < nfds; ++n) {
        int fd = events[n].data.fd;

        if(fd == socket) {

            int result = CM_NET_ERR;

            bio = ssl_store.get(fd, nullptr);
            if(nullptr == bio) {
                cm_log::error("Failed to find bio for fd");
                continue;
            }

            if(events[n].events & EPOLLIN) {
                result = service_input_event(fd);
                if(CM_NET_EOF == result) {
                    connected = false;
                    client_thread_ssl::remove_fd(fd);
                    //cm_net::delete_socket(epollfd, fd);
                    //cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection", fd));
                    return false;
                }
            }

            // handle peer shutdown
            if(events[n].events & EPOLLRDHUP) {
                // remove socket from interest list...
                if(CM_NET_OK == result) {
                    connected = false;
                    client_thread_ssl::remove_fd(fd);
                    //cm_net::delete_socket(epollfd, fd);
                    //cm_net::close_socket(fd);
                    cm_log::info(cm_util::format("%d: closed connection (EPOLLRDHUP).", fd));
                }
                cm_log::info(cm_util::format("%d: peer shutdown", fd));
                return false;
            }           
        }
    }

    return true;   
}

int cm_net::client_thread_ssl::service_input_event(int fd) {

    while(1) {
        
        int status = bio->service_io();
        if(status == CM_NET_AGAIN) {
            // back to caller for the next epoll_wait()
            return CM_NET_OK;
        }

        if(status == CM_NET_EOF) {
            // EOF - client disconnected
            return CM_NET_EOF;
        }

        if(status == CM_NET_ERR) {
            cm_net::err("service_io", errno);
            return CM_NET_ERR;
        }
    }
}

