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

int enable_reuseaddr(int fd);
int create_socket();
int server_socket(int host_port);
void close_socket(int fd);
int accept(int host_socket, std::string &info);
int connect(const std::string &host, int host_port, std::string &info);
void send(int socket, const std::string &msg);
void send(int socket, char *buf, size_t buf_size, const std::string &msg);
int recv(int socket, char *buf, size_t buf_size);
int recv_non_blocking(int socket, char *buf, size_t buf_size);

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

// client/server model
//======================================================================== 

class duplex_connection {

    struct tx_thread: public cm_thread::basic_thread {

        char sbuf[4096] = { '\0' };
        int socket;
        cm_queue::double_queue<std::string> tx_queue;

        size_t count = 0;
        std::string out;
        
        // tx_send
        bool process() {
            while(!tx_queue.empty()) {
                out = std::move(tx_queue.pop_front());
                cm_net::send(socket, sbuf, sizeof(sbuf), out);
                count += out.size();
                if(count % 4096 == 0) {
                    // tweak this to optimal
                    timespec delay = {0, 2000000}; // 2 ms
                    nanosleep(&delay, NULL);
                }
            }
            return true;
        }

        tx_thread(int s): socket(s) { start(); };
        ~tx_thread() { stop(); }
    };

    struct rx_thread: public cm_thread::basic_thread  {

        char rbuf[4096] = { '\0' };
        int socket;
        cm_queue::double_queue<std::string> rx_queue;

        size_t count = 0;
        std::string in;

        // rx_receive
        bool process() {
            int num_bytes = 0;
            // blocking, waits for socket data
            if(CM_NET_ERR != (num_bytes = cm_net::recv(socket, rbuf, sizeof(rbuf)))) {
                in = std::move(std::string(rbuf, num_bytes));
                rx_queue.push_back(in);
                count += num_bytes;
            }

            if(num_bytes == 0) {
                // disconnected, end this thread
                return false;
            }

            return num_bytes != CM_NET_ERR;
        }

        rx_thread(int s): socket(s) { start(); };
        ~rx_thread() { stop(); }
    };

    tx_thread *tx = nullptr;
    rx_thread *rx = nullptr;
    int socket;


    bool setup() {
        tx = new tx_thread(socket);
        rx = new rx_thread(socket);

        if(nullptr == rx) {
            if(nullptr != tx) delete tx;
            return false;
        }  

        return true;
    }

    void cleanup() {
        if(nullptr != tx) delete tx;
        if(nullptr != rx) delete rx;
    }

public:

    // improve this test
    bool is_done() { rx->is_done() && tx->is_done(); }

    bool has_rx_data() { return !rx->rx_queue.empty(); }
    bool has_tx_data() { return !tx->tx_queue.empty(); }

    size_t rx_count() { return rx->count; }
    size_t tx_count() { return tx->count; }


    duplex_connection(int s): socket(s) { setup(); }

    ~duplex_connection() { cleanup(); }

    void send(const std::string &s) {
        tx->tx_queue.push_back(s);
    }

    bool receive(std::string &s) {
        if(has_rx_data()) {
            s = rx->rx_queue.pop_front();
            return true;
        }
        return false;
    }

};

class client: public cm_thread::basic_thread {

protected:

    std::string host;
    int host_port;
    std::string info;
    
    int socket;
    duplex_connection *duplex = nullptr;

    bool connect() {
        // connect to host
        socket = cm_net::connect(host, host_port, info);
        if(-1 == socket) {
            return false;
        } 
  
        duplex = new duplex_connection(socket);
        if(nullptr == duplex) {
            cm_log::error("client: create duplex_connection failed");
            cm_net::close_socket(socket);
            return false;
        }
        return true;
    }
    
    bool setup() {

        if(!connect())  return false;

        cm_log::info(cm_util::format("client: connected to: %s (%d)",
            info.size() > 0 ? info.c_str(): "host?:serv?", socket));

        //wait for connection to stablize before we start sending data
        timespec delay = {0, 100000000};   // 0.1 seconds
        nanosleep(&delay, NULL);

        return true;
    }

    void cleanup() { 
        cm_net::close_socket(socket);
        if(nullptr != duplex) {
            delete duplex;
            duplex = nullptr;
        }
    }

    std::string in;

    bool process() {
        if(duplex->receive(in)) {
            on_receive(in);
        }

        // to-do: add thread monitor and shutdown check

        return true;
    }
    
public:
    client(const std::string _host, int _host_port):
        host(_host), host_port(_host_port) { start(); }
    ~client() {  stop();  }

    size_t rx_count() { return duplex->rx_count(); }
    size_t tx_count() { return duplex->tx_count(); }

    void send(const std::string &msg) { duplex->send(msg); }

    void send_request(const std::string &msg) {

        size_t pre_count = rx_count();

        send(msg);

        for(int count = 1000; rx_count() == pre_count; --count) {
            // delay for response
            timespec delay = {0, 6000000};   // 6 ms
            nanosleep(&delay, NULL);
        }
    }

    virtual void on_receive(const std::string &msg) = 0; 
};



class server_client: public cm_thread::basic_thread {

protected:
  

    duplex_connection *duplex = nullptr;

    int socket;
    std::string info;
   

    bool connect() {

        duplex = new duplex_connection(socket);
        if(nullptr == duplex) {
            cm_log::error("server_client: create duplex_connection failed");
            cm_net::close_socket(socket);
            return false;
        }
        return true;
    }
    
    bool setup() {

        if(!connect())  return false;

        cm_log::info(cm_util::format("server_client: connected to: %s (%d)",
            info.size() > 0 ? info.c_str(): "host?:serv?", socket));

        //wait for connection to stablize before we start sending data
        timespec delay = {0, 100000000};   // 0.1 seconds
        nanosleep(&delay, NULL);

        return true;
    }

    void cleanup() { 
        cm_net::close_socket(socket);
        if(nullptr != duplex) {
            delete duplex;
            duplex = nullptr;
        }
    }

    std::string in;

    bool process() {
        if(duplex->receive(in)) {
            on_receive(in);
        }

        // to-do: add thread monitor and shutdown check

        return true;
    }

void _send(const std::string &msg) { duplex->send(msg); }
    
public:
    server_client(int _socket, const std::string _info) {
        socket = _socket;
        info = _info;
        start();
    }
    ~server_client() {  stop();  }

    size_t rx_count() { return duplex->rx_count(); }
    size_t tx_count() { return duplex->tx_count(); }

    void send(const std::string &msg) {

        size_t pre_count = rx_count();

        _send(msg);

        for(int count = 1000; rx_count() == pre_count; --count) {
            // delay for response
            timespec delay = {0, 6000000};   // 6 ms
            nanosleep(&delay, NULL);
        }
    }

    virtual void on_receive(const std::string &msg) = 0; 
};


template<typename server_clientT>
class server: public cm_thread::basic_thread  {

protected:
   
    std::vector<std::unique_ptr<server_clientT>> connections;

    int host_port;
    int host_socket;
    std::string info;
  
    bool setup() {

        host_socket = cm_net::server_socket(host_port);
        if(-1 == host_socket) {
            return false;
        }
        return true;
    }

    void cleanup() { 
        cm_net::close_socket(host_socket);
    }

    bool process() {

        int socket = cm_net::accept(host_socket, info);
        if(-1 == socket) {
            cm_log::critical("accept failed in server thread, ending thread process");
            return false;
        }

        // start a thread to handle this new connection
        service_connection(socket, info);

        return true;
    }

    void service_connection(int socket, const std::string info) {

        cm_log::info(cm_util::format("server: connected to: %s (%d)",
            info.size() > 0 ? info.c_str(): "host?:serv?", socket));

        auto p = std::make_unique<server_clientT>(socket, info);
        connections.push_back(std::move(p));
       

        //cleanup connections list
        for(auto i = connections.begin(); i != connections.end(); i++) {
            if((*i)->is_done()) {
                connections.erase(i);
            }
        }
    }
    
public:
    server(int port): host_port(port) { start(); }
    ~server() { stop(); }
};


} // namespace cm_net

#endif
