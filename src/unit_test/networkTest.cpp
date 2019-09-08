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

        // connect to server
        if(CM_NET_ERR == (socket = cm_net::connect("127.0.0.1", 56000))) {
            cm_log::error("client failed to connect");
            return false;
        }

        cm_log::info("client_thread running: connected to server");

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


}

cm_log::file_logger server_log("./log/network_test.log");

void networkTest::test_network() {

    set_default_logger(&server_log);

    cm_log::info("test_network");

    cm_net::server_thread server(56000 /* port */, receive);
    CPPUNIT_ASSERT( server.is_started() == true );


    client_thread client;
    CPPUNIT_ASSERT( client.is_started() == true);
    

    sleep(3);
}

