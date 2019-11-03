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

#include "log.h"


#define SIO_OK 0
#define SIO_ERR -1

namespace cm_serial {


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

int sio_open(const char *device_name) {

    int fd = open(device_name, O_RDWR | O_NOCTTY | O_SYNC);
    if(fd < 0) {
        err("sio_open", errno);
        return SIO_ERR;
    }
    return fd;
}

int sio_init(int fd, int speed) {

    struct termios options;

    // get the current options 
    if(tcgetattr(fd, &options) < 0) {
        err("sio_init: tcgetattr", errno);
        return SIO_ERR;
    }

    // set output speed
    if(cfsetospeed(&options, (speed_t)speed) < 0) {
        err("sio_init: cfsetospeed", errno);
        return SIO_ERR;
    }

    // set input speed
    if(cfsetispeed(&options, (speed_t)speed) < 0) {
        err("sio_init: cfsetispeed", errno);
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
        err("sio_init: tcsetattr", errno);
        return SIO_ERR;
    }

    return SIO_OK;
}

int sio_read(int fd, void *buf, size_t sz) {

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

int sio_write(int fd, const unsigned char *buf, size_t sz) {

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

} // namespace cm_serial

#endif
