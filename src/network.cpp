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


int cm_net::enable_reuseaddr(int fd) {
    int enable = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
        cm_net::err("setsockopt: SO_REUSEADDR", errno);
        return CM_NET_ERR;
    }
    return CM_NET_OK;
}

int cm_net::create_socket() {

    int fd = socket(AF_INET, SOCK_STREAM, 0 /*protocol*/);
     if(-1 == fd) {
        cm_net::err("create socket", errno);
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

int cm_net::server_socket(int host_port) {

    // create host socket
    
    int host_socket = cm_net::create_socket();
    if(CM_NET_ERR == host_socket) {
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
   
    cm_log::info(cm_util::format("server: listening on %d", host_port)); 

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

    while(-1 == (fd = ::accept(host_socket, (sockaddr *) &client_hint, &client_sz))) {
        if(errno != EINTR) {
            cm_net::err("error on accept", errno);
            return CM_NET_ERR;
        }
        cm_log::info("accept interrupted, looping to retry");
        /* blocking accept interrupted, loop for retry */
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

    cm_log::info(cm_util::format("accept: new socket = %d", fd)); 

    info.assign(info_buf);

    return fd;
}

int cm_net::connect(const std::string &host, int host_port, std::string &info) {

    // create socket

    int fd = cm_net::create_socket();
    if(CM_NET_ERR == fd) {
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

int cm_net::read(int fd, char *buf, size_t sz) {

    // Read until sz bytes have been read (or error/EOF).
    ssize_t num_bytes, total_bytes = 0;
    while(total_bytes != sz) {
        num_bytes = read(fd, buf, sz - total_bytes);
        if (num_bytes == 0) return total_bytes;     // EOF
        if (num_bytes == -1) return -1;             // error
        total_bytes += num_bytes;
        buf += num_bytes;
    }

    return total_bytes;
}


int cm_net::write(int fd, char *buf, size_t sz) {

    // Write until sz bytes have been written (or error/EOF).
    ssize_t num_bytes, total_bytes = 0;
    while(total_bytes != sz) {
        num_bytes = write(fd, buf, sz - total_bytes);
        if (num_bytes == 0) return total_bytes;     // EOF
        if (num_bytes == -1) return -1;             // error
        total_bytes += num_bytes;
        buf += num_bytes;
    }

    return total_bytes;
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


//////////////////// server_thread //////////////////////////////

cm_net::server_thread::server_thread(int port, CM_NET_RECEIVE(fn)):
     host_port(port), receive_fn(fn) {

    // start processing thread
    start();
}

cm_net::server_thread::~server_thread() {
    // stop processing thread
    stop();
}


bool cm_net::server_thread::setup() {

    host_socket = cm_net::server_socket(host_port);
    if(-1 == host_socket) {
        return false;
    }
    return true;
}

void cm_net::server_thread::cleanup() {

    cm_net::close_socket(host_socket);
}

int cm_net::server_thread::accept() {
    // wait for and accept a client connection
    return cm_net::accept(host_socket, info);
}


bool cm_net::server_thread::process() {

    int socket = accept();
    if(-1 == socket) {
        cm_log::critical("accept failed in server thread, ending thread process");
        return false;
    }

    // start a thread to handle this new connection
    service_connection(socket, info);

    return true;
}


void cm_net::server_thread::service_connection(int socket, const std::string info) {

    auto p = create_connection_thread(socket, info);
    connections.push_back(std::move(p));

    cm_log::info(cm_util::format("server: client connection: %s", info.size() > 0 ? info.c_str():
        "host?:serv?"));

    //cleanup connections list
    for(auto i = connections.begin(); i != connections.end(); i++) {
        if((*i)->is_done()) {
            connections.erase(i);
        }
    }

}

///////////////////// connection_thread //////////////////////////

bool cm_net::connection_thread::setup() {
    return true;
}

void cm_net::connection_thread::cleanup() {
    cm_net::close_socket(socket);
}

cm_net::connection_thread::connection_thread(int _socket, const std::string _info,
    CM_NET_RECEIVE(fn)): socket(_socket), receive_fn(fn) {
    info = _info;
    // start processing thread
    start();
}

cm_net::connection_thread::~connection_thread() {
    // stop processing thread
    stop();
}


void cm_net::connection_thread::send(const std::string &msg) {
    cm_net::send(socket, sbuf, sizeof(sbuf), msg);
}

void cm_net::connection_thread::receive(const char *buf, size_t sz) {
    receive_fn(socket, buf, sz);
}

bool cm_net::connection_thread::process() {

    // block until next request received
    int num_bytes = cm_net::recv(socket, rbuf, sizeof(rbuf));
    if(CM_NET_ERR == num_bytes) {
        return false;
    }

    if(0 == num_bytes) {
            // client disconnected
            return false;
    }

    receive(rbuf, num_bytes);

    return true;
}

//////////////////// client_thread //////////////////////////////

cm_net::client_thread::client_thread(const std::string _host, int port, CM_NET_RECEIVE(fn)):
     host(_host), host_port(port), receive_fn(fn) {
    // start processing thread
    start();
}

cm_net::client_thread::~client_thread() {
    // stop processing thread
    stop();
}

int cm_net::client_thread::connect() {
    return cm_net::connect(host, host_port, info);
}

bool cm_net::client_thread::setup() {

    // connect to host
    socket = cm_net::client_thread::connect();
    if(-1 == socket) {
        return false;
    }
    
    connection = std::move(new cm_net::connection_thread(socket, info, receive_fn));

    cm_log::info(cm_util::format("client: connected to: %s", info.size() > 0 ? info.c_str():
        "host?:serv?"));

    //wait for connection to stablize before we start sending data
    timespec delay = {0, 100000000};   // 0.1 seconds
    nanosleep(&delay, NULL);

    return true;
}

void cm_net::client_thread::cleanup() {

    cm_net::close_socket(socket);
     if(connection != nullptr) {
        delete connection;
        connection = nullptr;
    }
}

bool cm_net::client_thread::process() {
    // we end this thread when our connection thread has ended
    return connection->is_done() == false;
}
