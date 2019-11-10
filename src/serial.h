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

#ifndef __SERIAL_H
#define __SERIAL_H

#pragma once

#include <string.h>
#include <sys/types.h>
#include <stdio.h>    // Standard input/output definitions 
#include <unistd.h>   // UNIX standard function definitions 
#include <fcntl.h>    // File control definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <string.h>   // String function definitions 
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "log.h"
#include "thread.h"


#define SIO_OK 0
#define SIO_ERR -1

#define MAX_EVENTS  64

namespace cm_sio {


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

inline int sio_open(const char *device_name) {

    int fd = open(device_name, O_RDWR | O_NOCTTY | O_SYNC);
    if(fd < 0) {
        cm_sio::err(cm_util::format("sio_open: %s", device_name), errno);
        return SIO_ERR;
    }
    return fd;
}

inline int sio_init(int fd, int speed) {

    struct termios options;

    // get the current options 
    if(tcgetattr(fd, &options) < 0) {
        cm_sio::err("sio_init: tcgetattr", errno);
        return SIO_ERR;
    }

    // set output speed
    if(cfsetospeed(&options, (speed_t)speed) < 0) {
        cm_sio::err("sio_init: cfsetospeed", errno);
        return SIO_ERR;
    }

    // set input speed
    if(cfsetispeed(&options, (speed_t)speed) < 0) {
        cm_sio::err("sio_init: cfsetispeed", errno);
        return SIO_ERR;        
    }

    // setup for raw input
    options.c_cflag |= (CLOCAL | CREAD); // read on, ignore ctrl lines
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;         /* 8-bit characters */
    options.c_cflag &= ~PARENB;     /* no parity bit */
    options.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    options.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */
   
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    options.c_oflag &= ~OPOST;

    // http://unixwiz.net/techtips/termios-vmin-vtime.html
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 5;   // deciseconds (10ths of a second)

    // set the options 
    if(tcsetattr(fd, TCSANOW, &options) < 0) {
        cm_sio::err("sio_init: tcsetattr", errno);
        return SIO_ERR;
    }

    return SIO_OK;
}

inline int sio_read(int fd, char *buf, size_t sz) {

    // Read until sz bytes have been read (or error/timeout).
    ssize_t num_bytes, total_bytes = 0;
    while(total_bytes != sz) {
        num_bytes = ::read(fd, buf, sz - total_bytes);
        if (num_bytes <= 0) break;     // error/timedout reached
        total_bytes += num_bytes;
        buf += num_bytes;
    }

    if(total_bytes > 0) {
        return total_bytes;
    } 

    return num_bytes < 0 ? -1 : total_bytes;
}

inline int sio_write(int fd, const char *buf, size_t sz) {

    // Write until sz bytes have been written (or error/timeout).
    ssize_t num_bytes, total_bytes = 0;
    while(total_bytes != sz) {
        num_bytes = ::write(fd, buf, sz - total_bytes);
        if (num_bytes <= 0) break;    // error/timedout reached
        total_bytes += num_bytes;
        buf += num_bytes;
    }

    if(total_bytes > 0) {
        tcdrain(fd);    // delay for output
        return total_bytes;
    } 

    return num_bytes < 0 ? -1 : total_bytes;
}


/////////////////////// event-driven I/O /////////////////////////////////

inline int epoll_create() {
    int fd = epoll_create1(0);
    if(-1 == fd) {
        cm_sio::err("epoll_create", errno);
        return SIO_ERR;
    }
    return fd;
}

inline int add_fd(int epollfd, int fd, uint32_t flags) {
    struct epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        cm_sio::err(cm_util::format("%d: epoll_ctl:ADD", fd), errno);
        return SIO_ERR;
    }
    return SIO_OK;
}

inline int modify_fd(int epollfd, int fd, uint32_t flags) {
    struct epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        cm_sio::err(cm_util::format("%d: epoll_ctl:MOD", fd), errno);
        return SIO_ERR;
    }
    return SIO_OK;
}

inline int delete_fd(int epollfd, int fd) {
    struct epoll_event ev;
    ev.events = 0;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        cm_sio::err(cm_util::format("%d: epoll_ctl:DEL", fd), errno);
        return SIO_ERR;
    }
    return SIO_OK;
}

///////////////////////// sio_server ///////////////////////////////////

#define sio_callback(fn) void (*fn)(int fd, const char *buf, size_t sz)

class sio_server: public cm_thread::basic_thread  {

protected:

    std::vector<std::string> ports;
    std::vector<int> port_fds;

    sio_callback(receive_fn) = nullptr;

    char rbuf[128] = { '\0' };

    int epollfd;
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, timeout = -1; // ms timeout    

    bool port_setup(const char *port, int speed) {

        int listen_fd = cm_sio::sio_open(port);
        if(-1 == listen_fd) {
            return false;
        }

        if(sio_init(listen_fd, speed) != SIO_OK) {
            close(listen_fd);
            return false;
        }

        if(-1 == cm_sio::add_fd(epollfd, listen_fd, EPOLLIN | EPOLLET)) {
            close(listen_fd);
            return false;
        }

        port_fds.push_back(listen_fd);

        return true;
    }


    bool setup() {

        epollfd = cm_sio::epoll_create();
        if(-1 == epollfd) {
            return false;
        }

        for(auto &port: ports) {
            if(port_setup(port.c_str(), B9600)) {
                cm_log::info(cm_util::format("listening to %s", port.c_str()));
            }
            else {
                cm_log::warning(cm_util::format("ignoring %s", port.c_str()));
            }
        }

        return true;
    }

    void cleanup() {

        for(auto fd: port_fds) {
            cm_sio::delete_fd(epollfd, fd); 
            close(fd);
        }
    }

    bool process() {

        // fetch fds that are ready for I/O...
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
        if(-1 == nfds) {
            cm_sio::err("epoll_wait", errno);
            return false;
        }

        // process the ready fds
        for(int n = 0; n < nfds; ++n) {

            int fd = events[n].data.fd;

            //if(fd != epollfd) {
                int result = 0;
                if(events[n].events & EPOLLIN) {
                    result = cm_sio::sio_server::service_input_event(fd);
                    if(SIO_ERR == result) {
                        cm_sio::err("sio_server: service_input_event", errno);
                    }
                }

            //}
        }

        return true;
    }

    int service_input_event(int fd) {
    
        cm_log::trace("service_input_event");
        int read;

        //while(1) {
            
            read = sio_read(fd, rbuf, sizeof(rbuf));

            if(read > 0) {
                receive_fn(fd, rbuf, read);
            }

            if(read <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // back to caller for the next epoll_wait()
                return SIO_OK;
            }

            if(read < 0) {
                cm_sio::err("sio_server: service_io", errno);
                return SIO_ERR;
            }
        //}

        return SIO_OK;
    }
    
public:
    sio_server(const std::vector<std::string> &_ports, sio_callback(fn)): ports(_ports),
     receive_fn(fn) {
        // start processing thread
        start();
    }

    ~sio_server() {
        // stop processing thread
        stop();
    }

};

} // namespace cm_sio

#endif
