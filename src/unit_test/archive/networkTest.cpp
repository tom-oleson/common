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

// receive function called by server connection_threads
void client_receive(int socket, const char *buf, size_t sz) {

    cm_log::info("received response:");
    cm_log::hex_dump(cm_log::level::info, buf, sz, 16);
}

struct unit_client: public cm_net::client_thread {

    int count = 0;

    unit_client():
    cm_net::client_thread("127.0.0.1", 56000, client_receive) { }
    
    bool process() {

        for(int x = 0; x < 10000; x++) {
            if(++count % 10000 == 0) {
                    send(cm_util::format("count = [%d]", count));
                  
                    //timespec delay = {0, 100000000};   // 100 ms
                    timespec delay = {0, 2000000};   // 2 ms
                    nanosleep(&delay, NULL);

                    //receieve response
                }
        }
        return count < 1000000;
    }
};


// receive function called by server connection_threads
void server_receive(int socket, const char *buf, size_t sz) {

    cm_log::info("received request:");
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
    sleep(12);
   
}


struct unit_user_client: public cm_net::client {

    unit_user_client(const std::string &host, int port):
        cm_net::client(host, port) { }

 
    virtual void on_receive(const std::string &msg) {
        process_response(msg);
    }

    void process_response(const std::string &msg) {

        cm_log::info("response:");
        cm_log::hex_dump(cm_log::level::info, msg.c_str(), msg.size(), 16);

    }
};


struct unit_server_client: public cm_net::server_client {

    unit_server_client(int socket, const std::string _info):
        server_client(socket, info) {}

    virtual void on_receive(const std::string &msg) {
        cm_log::info("request:");
        cm_log::hex_dump(cm_log::level::info, msg.c_str(), msg.size(), 16);
    }

};

void networkTest::test_network2() {

    cm_log::file_logger server_log("./log/network_test.log");
    set_default_logger(&server_log);
    //server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}> ${file}:${line}: ${msg}");
    server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");
    
    // startup tcp server
    cm_net::server<unit_server_client> server(56000 /* port */);
    //cm_net::server_thread server(56000 /* port */, server_receive);
    CPPUNIT_ASSERT( server.is_started() == true );
    sleep(1);


    unit_user_client my_client("127.0.0.1", 56000);
   
    my_client.send("hello");
    sleep(1);

    my_client.send_request("hello");
    my_client.send_request("status");
    
    cm_log::info(cm_util::format("rx_count = %ld", my_client.rx_count()));

    CPPUNIT_ASSERT( my_client.rx_count() > 0);
    
    sleep(4);
}
