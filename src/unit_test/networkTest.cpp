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


void client_receive(int socket, const char *buf, size_t sz) {

    cm_log::info(cm_util::format("%d: received response:", socket));
    cm_log::hex_dump(cm_log::level::info, buf, sz, 16);
}

struct unit_client: public cm_net::client_thread {

    int count = 0;

    unit_client():
    cm_net::client_thread("127.0.0.1", 56000, client_receive) { }
    
    bool process() {

        for(int x = 0; x < 10000; x++) {
            if(++count % 10000 == 0) {
                    send(cm_util::format("count=[%d]", count));
                  
                    //timespec delay = {0, 100000000};   // 100 ms
                    timespec delay = {0, 2000000};   // 2 ms
                    nanosleep(&delay, NULL);

                    //receieve response
                }
        }
        return count < 1000000;
    }
};


void server_receive(int socket, const char *buf, size_t sz) {

    cm_log::info(cm_util::format("%d: received request:", socket));
    cm_log::hex_dump(cm_log::level::info, buf, sz, 16);

    std::string response("OK");
    
    if(strncmp(buf, "status", 6) == 0) {
        response = std::move(std::string("status: active"));
    }

    cm_net::send(socket, response);

}

void networkTest::test_network() {

    cm_log::file_logger server_log("./log/network_test.log");
    set_default_logger(&server_log);
    //server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}> ${file}:${line}: ${msg}");
    server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");
    
    // startup tcp server thread
    cm_net::server_thread server(56000 /* port */, server_receive);
    CPPUNIT_ASSERT( server.is_started() == true );

    // run multiple client threads to feed data to server thread
    // these must not go in containers, the constructors will
    // get called multiple times

    unit_client client1;
    unit_client client2;
    unit_client clinet3;
    unit_client clinet4;
    unit_client clinet5;
    unit_client clinet6;
    unit_client clinet7;
    unit_client clinet8;
    
    sleep(3);
   
}
