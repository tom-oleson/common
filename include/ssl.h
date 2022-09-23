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

#ifndef __SSL_H
#define __SSL_H

#pragma once

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "log.h"
#include "config.h"

#define CM_SSL_FAILURE -1

namespace cm_ssl {

int print_errors(const char *str, size_t len, void *u);

void init_openssl();
void cleanup_openssl();
SSL_CTX *ctx_create();
void ctx_free(SSL_CTX *ctx);
void ctx_configure(SSL_CTX *ctx);
void client_ctx_configure(SSL_CTX *ctx);
X509 *ssl_get_peer_certificate(const SSL *ssl);
void ssl_X509_free(X509 *cert);
SSL *ssl_create(SSL_CTX *ctx);
void ssl_shutdown(SSL *ssl);
void ssl_free(SSL *ssl);
int ssl_set_fd(SSL *ssl, int fd);
int ssl_accept(SSL *ssl);
int ssl_connect(SSL *ssl);
int ssl_read(SSL *ssl, void *buf, int num);
int ssl_write(SSL *ssl, const void *buf, int num);
void ssl_set_accept_state(SSL *ssl);
void ssl_set_connect_state(SSL *ssl);
int ssl_is_server(SSL *ssl);
const char *ssl_get_version(SSL *ssl);
int ssl_get_error(SSL *ssl, int ret);
int ssl_is_init_finished(SSL *ssl);
std::string ssl_error_string(unsigned long e);
void ssl_set_bio(SSL *ssl, BIO *rbio, BIO *wbio);


inline const char *err_reason_error_string(unsigned long e) {
    return ERR_reason_error_string(e);
}

/*
//////////////////////////////////////////////////////////////////////////////

  +-----+                                    +-----+
  |  s  |--> read(fd) --> BIO_write(rbio) -->|  S  |--> SSL_read(ssl)  --> IN
  |  o  |                                    |  S  |
  |  c  |                                    |  L  |
  |  k  |<-- write(fd) <-- BIO_read(wbio) <--|     |<-- SSL_write(ssl) <-- OUT
  +-----+                                    +-----+
        |<---------------------------------->|     |<------------------->|
        |          encrypted data            |     |      clear data     |

//////////////////////////////////////////////////////////////////////////////
*/

#define CM_SSL_OK 1
#define CM_SSL_EOF 0
#define CM_SSL_ERR -1
#define CM_SSL_AGAIN -2
#define CM_SSL_WANT_WRITE -3

#define ssl_receive_cb(fn) void (*fn)(const char *buf, size_t sz)

struct ssl_bio {

    int fd = -1;
    SSL *ssl = nullptr;
    BIO *rbio = nullptr;  // we write, SSL reads (in)
    BIO *wbio = nullptr;  // we read, SSL writes  (out)
    X509 *cert = nullptr;
    bool is_server = false;

    ssl_receive_cb(receive_fn) = nullptr;

    void setup() {

        rbio = BIO_new(BIO_s_mem());
        wbio = BIO_new(BIO_s_mem());

        //BIO_set_mem_eof_return(rbio, -1); 
        //BIO_set_mem_eof_return(wbio, -1);

        BIO_set_nbio(rbio, 1);
        BIO_set_nbio(wbio, 1);
  
        // NOTE: do not call SSL_set_fd(ssl, fd) for memory BIOs!


        if(is_server) {
            SSL_set_accept_state(ssl);
        }
        else {
            SSL_set_connect_state(ssl);
        }

        SSL_set_bio(ssl, rbio, wbio);
    }

    void cleanup() {
        ssl_free(ssl);
    }

    ssl_bio(SSL *_ssl, int _fd, bool _is_server, ssl_receive_cb(_fn)):
     ssl(_ssl), fd(_fd), is_server(_is_server), receive_fn(_fn) {
        setup();
    }

    ~ssl_bio() {
        cleanup();
    }

    // write bytes to socket
    ssize_t socket_write(char *buf, ssize_t sz) {
        cm_log::trace(cm_util::format("%d: socket_write", fd));
        return ::write(fd, buf, sz);
    }

    // read bytes from socket
    ssize_t socket_read(char *buf, ssize_t sz) {
        cm_log::trace(cm_util::format("%d: socket_read", fd));
        return ::read(fd, buf, sz);
    }

    int ssl_status(int ret) {

        switch (ret) {
            case SSL_ERROR_NONE:
              return CM_SSL_OK;

            case SSL_ERROR_WANT_WRITE:
                return CM_SSL_WANT_WRITE;

            case SSL_ERROR_WANT_READ:
              return CM_SSL_AGAIN;

            case SSL_ERROR_ZERO_RETURN:
               return CM_SSL_EOF;

            case SSL_ERROR_SYSCALL:
            default:
              return CM_SSL_ERR;
        }
    }   

    // read encrypted bytes from socket and write to rbio
    int do_bio_write() {

        cm_log::trace("do_bio_write");

        char buf[1024] = {'\0'};
        ssize_t read, written, status;

        do {
            read = socket_read(buf, sizeof(buf));
            cm_log::trace(cm_util::format("socket_read: %d", read)); 
            if(read > 0) {
                written = BIO_write(rbio, buf, read);
                status = ssl_status(SSL_get_error(ssl, written));
                cm_log::trace(cm_util::format("bio_write: %d", written));

                if(!SSL_is_init_finished(ssl)) 
                    do_handshake();

                if(written <= 0) {
                    return status;
                }
            }
            else {
                if(errno == EAGAIN || errno == EWOULDBLOCK) return CM_SSL_AGAIN;
                if(read == 0) return CM_SSL_EOF;
                return CM_SSL_ERR;
            }
        } while(read > 0);
        return read;
    }

    X509 *do_get_peer_certificate() {
        return SSL_get_peer_certificate(ssl);
    }

    // read encrypted bytes from wbio and write to socket
    int do_bio_read() {

        cm_log::trace("do_bio_read");

        char buf[1024] = {'\0'};
        int read, written, status;
        do {
            read = BIO_read(wbio, buf, sizeof(buf));
            status = ssl_status(SSL_get_error(ssl, read));
            cm_log::trace(cm_util::format("bio_read: %d", read)); 
            if(read > 0) {
                written = socket_write(buf, read);
                cm_log::trace(cm_util::format("socket_write: %d", written)); 
                if(written <= 0) {
                    if(errno == EAGAIN || errno == EWOULDBLOCK) return CM_SSL_AGAIN;
                    if(written == 0) return CM_SSL_EOF;
                    return CM_SSL_ERR;
                }
            }
            else {
                return status;
            }
        } while(read > 0);
        return read;
    }

    // called by client to begin SSL handshake
    int do_handshake() {

        cm_log::trace("do_handshake");

        int n = SSL_do_handshake(ssl);
        ERR_print_errors_fp(stderr);
        int status = ssl_status(SSL_get_error(ssl, n));
        cm_log::trace(cm_util::format("do_handshake: status: %d", status)); 

        //if(status == CM_SSL_AGAIN || status == CM_SSL_WANT_WRITE) 
            do_bio_read();

        return status;
    }

    int do_ssl_read() {

        cm_log::trace("do_ssl_read");

        char buf[1024] = {'\0'};
        ssize_t read, status;  

        do {
            read = SSL_read(ssl, buf, sizeof(buf));
            status = ssl_status(SSL_get_error(ssl, read));
            if(read > 0) {
                cm_log::trace("call receive_fn");
                receive_fn(buf, (size_t) read);
            }
            else {
                cm_log::trace(cm_util::format("do_ssl_read: status: %d", status)); 
                return status;
            }

        } while(read > 0);
        return read;
    }

    ssize_t do_ssl_write(const char *buf, size_t sz) {

        cm_log::trace("do_ssl_write");
        
        ssize_t written, status, total_written = 0;  

        do {
            written = SSL_write(ssl, buf, sz);
            status = ssl_status(SSL_get_error(ssl, written));
            if(written > 0) {
                total_written += written;
                status = do_bio_read();
                //status = ssl_status(SSL_get_error(ssl, n));
                if(status == CM_SSL_EOF) return CM_SSL_EOF;
                if(status < 0) return total_written;
            }
            else {
                cm_log::trace(cm_util::format("do_ssl_write: status: %d", status)); 
                return status;
            }

        } while(written > 0);
        return total_written;
    }

    // service I/O events - called in event loop to move the data
    // Process OUTPUT:
    // 1. do_bio_read (write encrypted data to socket)
    // Process INPUT:
    // 1. do_bio_write (read encrypted data from socket)
    // 2. do_ssl_read (read clear data and call receive_fn)
    // 
    int service_io() {

        int status;
       
        status = do_bio_read();
        if(status == CM_SSL_EOF) return CM_SSL_EOF;

        status = do_bio_write();
        if(status == CM_SSL_EOF) return CM_SSL_EOF;

        status = do_ssl_read();
        if(status == CM_SSL_EOF) return CM_SSL_EOF;

        return status;
    }
};

} // namespace cm_ssl

#endif
