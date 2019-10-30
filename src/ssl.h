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
int ssl_is_init_finished(const SSL *ssl);
std::string ssl_error_string(unsigned long e);
void ssl_set_bio(SSL *ssl, BIO *rbio, BIO *wbio);


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


#define receive_cb(fn) void (*fn)(SSL *ssl, const char *buf, size_t sz)

struct ssl_bio {

    SSL *ssl = nullptr;
    BIO *rbio = nullptr;  // we write, SSL reads (in)
    BIO *wbio = nullptr;  // we read, SSL writes  (out)
    bool is_server = false;

    receive_cb(receive_fn) = nullptr;

    void setup() {

        rbio = BIO_new(BIO_s_mem());
        wbio = BIO_new(BIO_s_mem());

        if(nullptr != rbio && nullptr != wbio) {
            BIO_set_mem_eof_return(rbio, -1); 
            BIO_set_mem_eof_return(wbio, -1);

            SSL_set_bio(ssl, rbio, wbio);
        }

        if(is_server) {
            SSL_set_accept_state(ssl);
        }
        else {
            SSL_set_connect_state(ssl);
        }
    }

    void cleanup() {

    }

    ssl_bio(SSL *_ssl, bool _is_server, receive_cb(_fn)):
     ssl(_ssl), is_server(_is_server), receive_fn(_fn) {
        setup();
    }

    ~ssl_bio() {
        cleanup();
    }

    // write bytes to socket
    ssize_t socket_write(char *buf, ssize_t sz) {
        return ::write(SSL_get_wfd(ssl), buf, sz);
    }

    // read bytes from socket
    ssize_t socket_read(char *buf, ssize_t sz) {
        return ::read(SSL_get_rfd(ssl), buf, sz);
    }

    // read encrypted bytes from socket and write to rbio
    int do_bio_write() {
        char buf[1024] = {'\0'};
        ssize_t n = 0, nb = 0, status = 0;
        do {
            n = socket_read(buf, sizeof(buf));
            if(n > 0) {
                if((nb = BIO_write(rbio, buf, n)) <= 0) {
                    status = SSL_get_error(ssl, nb);
                    if(!BIO_should_retry(rbio)) return -1;
                }
                if(!SSL_is_init_finished(ssl)) {
                    if(do_handshake() < 0) return -1;
                    if(!SSL_is_init_finished(ssl)) return 0;
                }
            }
        } while(n > 0);
        return status;
    }

    // read encrypted bytes from wbio and write to socket
    int do_bio_read() {
        char buf[1024] = {'\0'};
        int n = 0, status = 0;
        do {
            n = BIO_read(wbio, buf, sizeof(buf));
            status = SSL_get_error(ssl, n);
            if(n > 0) {
                if(socket_write(buf, n) < 0) {
                    if(errno != EAGAIN && errno != EWOULDBLOCK) return -1;
                    return status;
                }
            }
            else {
                if(!BIO_should_retry(wbio)) return -1;
            }
        } while(n > 0);
        return status;
    }

    // called by client to begin SSL handshake
    int do_handshake() {
        int n = SSL_do_handshake(ssl);
        int status = SSL_get_error(ssl, n);
        if(n <= 0) {
            if(status == SSL_ERROR_WANT_READ ||
                status == SSL_ERROR_WANT_WRITE) {
                return do_bio_read();
            }
        }
        return status;
    }

    int do_ssl_read() {
        char buf[1024] = {'\0'};
        ssize_t n = 0, status = 0;  

        do {
            n = SSL_read(ssl, buf, sizeof(buf));
            status = status = SSL_get_error(ssl, n);
            if(n > 0) {
                receive_fn(ssl, buf, (size_t) n);
            }
        } while(n > 0);
        return status;
    }

    // service I/O events - called in event loop to move the data
    // Process OUTPUT:
    // 1. do_bio_read (write encrypted data to socket)
    // Process INPUT:
    // 1. do_bio_write (read encrypted data from socket)
    // 2. do_ssl_read (read clear data and call receive_fn)
    // 
    int service_io() {

        // process INPUT
        int pending = BIO_ctrl_pending(wbio);
        if(pending > 0) {
            if(do_bio_read() < 0) {

            }
        }

        // process OUTPUT
        if(do_bio_write() == 0) {
            do_ssl_read();
        }
    }
};


} // namespace cm_ssl

#endif
