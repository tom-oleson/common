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

#include "ssl.h"


int cm_ssl::print_errors(const char *str, size_t len, void *u) {
    (void) u;
    cm_log::error(std::string(str, len));
}

void cm_ssl::init_openssl() {

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

void cm_ssl::cleanup_openssl() {

    ERR_free_strings();
    EVP_cleanup();
}

SSL_CTX *cm_ssl::ctx_create() {

    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (nullptr == ctx) {
        cm_log::error("Unable to create SSL context");
        ERR_print_errors_cb(cm_ssl::print_errors, NULL);
        exit(CM_SSL_FAILURE);
    }
    return ctx;
}

void cm_ssl::ctx_free(SSL_CTX *ctx) {
    SSL_CTX_free(ctx);
}

void cm_ssl::ctx_configure(SSL_CTX *ctx) {

    SSL_CTX_set_ecdh_auto(ctx, 1);
        
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_cb(cm_ssl::print_errors, NULL);
        exit(CM_SSL_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_cb(cm_ssl::print_errors, NULL);
        exit(CM_SSL_FAILURE);
    }

    if (!SSL_CTX_check_private_key (ctx)) {
        cm_log::error("Private key does not match the certificate");
        exit(CM_SSL_FAILURE);
    }

    //for non-blocking sockets to work?
    SSL_CTX_set_mode(ctx,
          SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER 
        | SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_AUTO_RETRY 
        | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3
        );
}

SSL *cm_ssl::ssl_create(SSL_CTX *ctx) {
    
    SSL *ssl = SSL_new(ctx);
    if (nullptr == ssl) {
        cm_log::error("Unable to create SSL structure");
        ERR_print_errors_cb(cm_ssl::print_errors, NULL);
    }
    return ssl;
}

void cm_ssl::ssl_shutdown(SSL *ssl) {
    SSL_shutdown(ssl);
}

void cm_ssl::ssl_free(SSL *ssl) {
    SSL_free(ssl);
}

int cm_ssl::ssl_set_fd(SSL *ssl, int fd) {
    
    int res = SSL_set_fd(ssl, fd);
    if (res <= 0) {
        cm_log::error("Unable to set SSL fd");
        ERR_print_errors_cb(cm_ssl::print_errors, NULL);
    }
    return res;
}

int cm_ssl::ssl_accept(SSL *ssl) {
    return SSL_accept(ssl);
}

int cm_ssl::ssl_connect(SSL *ssl) {
    return SSL_connect(ssl);
}

int cm_ssl::ssl_read(SSL *ssl, void *buf, int num) {
    return SSL_read(ssl, buf, num);
}

int cm_ssl::ssl_write(SSL *ssl, const void *buf, int num) {
    return SSL_write(ssl, buf, num);
}

void cm_ssl::ssl_set_accept_state(SSL *ssl) {
    SSL_set_accept_state(ssl);
}

void cm_ssl::ssl_set_connect_state(SSL *ssl) {
    SSL_set_connect_state(ssl);
}

int cm_ssl::ssl_is_server(SSL *ssl) {
    return SSL_is_server(ssl);
}

int cm_ssl::ssl_get_error(SSL *ssl, int ret) {
    return SSL_get_error(ssl, ret);
}

const char *cm_ssl::ssl_get_version(SSL *ssl) {
    return SSL_get_version(ssl);
}
