/**********************************************************************
*
* networkTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "networkTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( networkTest );

struct client_thread: public cm_thread::basic_thread {

    int count = 0;
    int socket = -1;
    char sbuf[4096] = { '\0' };

    client_thread() { start(); }

    void send(const std::string msg) {
        cm_net::send(socket, sbuf, sizeof(sbuf), msg);
    }

    bool setup() {

        std::string info;

        // connect to server
        if(CM_NET_ERR == (socket = cm_net::connect("127.0.0.1", 56000, info))) {
            cm_log::error("client failed to connect");
            return false;
        }

        cm_log::info(cm_util::format("client_thread running: connected to server: %s", info.c_str()));

        //wait for connection to stablize before we start sending data

        timespec delay = {0, 100000000};   // 0.1 seconds
        nanosleep(&delay, NULL);

        return true;
    }

    void cleanup() {

        cm_net::close_socket(socket);
    }

    bool process() {

        for(int x = 0; x < 10000; x++) {
            if(++count % 10000 == 0)
                send(cm_util::format("count = [%d]", count));
        }
        return count < 1000000;
    }
};

// receive function called by server connection_threads
void receive(const char *buf, size_t sz) {

    cm_log::info(std::string(buf, sz));

    char hex_buf[1024];
    bzero(hex_buf, sizeof(hex_buf));
    cm_util::bin2hex((unsigned char *) buf, sz, hex_buf, sizeof(hex_buf), true);
    cm_log::info(cm_util::format("%s", hex_buf));

}

void networkTest::test_network() {

    cm_log::file_logger server_log("./log/network_test.log");
    set_default_logger(&server_log);
    server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}> ${file}:${line}: ${msg}");
    
    // startup tcp server thread
    cm_net::server_thread server(56000 /* port */, receive);
    CPPUNIT_ASSERT( server.is_started() == true );

    // run multiple client threads to feed data to server thread
    client_thread client[6];

    sleep(4);
}

