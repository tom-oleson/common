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
                    send(cm_util::format("count=[%d]\n", count));
                  
                    // delay to allow server time to respond
                    timespec delay = {0, 100000000};   // 100 ms
                    nanosleep(&delay, NULL);

                }
        }

        bool finished = count < 1000000;
        return finished;
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
    server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");

    timespec start, now;
    clock_gettime(CLOCK_REALTIME, &start);
    
    // startup tcp server thread
    cm_net::single_thread_server server(56000 /* port */, server_receive);
    CPPUNIT_ASSERT( server.is_started() == true );

    // run multiple client threads to feed data to server thread
    // these must not go in containers, the constructors will
    // get called multiple times

    vector<unit_client *> clients;

    for(int n = 0; n < 50; ++n) {
        unit_client *p = new unit_client();
        CPPUNIT_ASSERT( p->is_valid() );
        clients.push_back(p);

        // let new client thread get stable!!!
        // if we create them too quickly, things go south...
        timespec delay = {0, 50000000};   // 50 ms
        nanosleep(&delay, NULL);
    }

    // wait for all the threads to finish
    for(auto p: clients) {
        while( p->is_valid() && !p->is_done() ) {
            timespec delay = {0, 10000000};   // 10 ms
            nanosleep(&delay, NULL);
        }
        delete p;
    }
    
   clock_gettime(CLOCK_REALTIME, &now);
   double total = cm_time::duration(start, now);

   cm_log::info(cm_util::format("total time: %7.4lf secs", total));
}

void request_handler(void *arg) {

    cm_net::input_event *event = (cm_net::input_event *) arg;
    std::string request = event->msg;
    int socket = event->fd;

    // if this is a connect event
    if(event->connect) {
        return;        
    }

    // if this is an EOF event (client disconnected)
    if(event->eof) {
        return;
    }    

    cm_log::info(cm_util::format("%d: received request:", socket));
    cm_log::hex_dump(cm_log::level::info, request.c_str(), request.size(), 16);

    std::string response("OK");
   

    cm_net::send(socket, response);
}

void request_dealloc(void *arg) {
    delete (cm_net::input_event *) arg;
}

void networkTest::test_network_thread_pool() {

    cm_log::file_logger server_log("./log/network_thread_pool_test.log");
    set_default_logger(&server_log);
    server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");

    // initialize random seed
    srand (time(NULL));

    // create thread pool that will do work for the server
    cm_thread::pool thread_pool(6);

    timespec start, now;
    clock_gettime(CLOCK_REALTIME, &start);

    // startup tcp server
    cm_net::pool_server server(56000, &thread_pool, request_handler,
        request_dealloc);

    CPPUNIT_ASSERT( server.is_started() == true );

    // run multiple client threads to feed data to server thread

    vector<unit_client *> clients;

    for(int n = 0; n < 50; ++n) {
        unit_client *p = new unit_client();
        CPPUNIT_ASSERT( p->is_valid() );
        clients.push_back(p);

        // allow new client thread to get stable in scheduler
        // new threads need time to setup, don't create them too quickly
        timespec delay = {0, 50000000};   // 50 ms
        nanosleep(&delay, NULL);
    }

    // wait for all the threads to finish
    for(auto p: clients) {
        while( p->is_valid() && !p->is_done() ) {
            timespec delay = {0, 10000000};   // 10 ms
            nanosleep(&delay, NULL);
        }
        delete p;
    }

     // wait for pool_server threads to complete all work tasks
    thread_pool.wait_all();

    clock_gettime(CLOCK_REALTIME, &now);
    double total = cm_time::duration(start, now);    
    cm_log::info(cm_util::format("total time: %7.4lf secs", total));

    thread_pool.log_counts();
}



void networkTest::test_client_connect() {

    cm_log::file_logger server_log("./log/client_connect_test.log");
    set_default_logger(&server_log);
    server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");

    timespec start, now;
    clock_gettime(CLOCK_REALTIME, &start);
    
    // startup tcp server thread
    //cm_net::single_thread_server server(56000 /* port */, server_receive);
    //CPPUNIT_ASSERT( server.is_started() == true );

    // try to start up a client that has no server to connect to
    unit_client client;
    CPPUNIT_ASSERT( !client.is_started() && client.is_done());

    // startup tcp server
    // create thread pool that will do work for the server
    cm_thread::pool thread_pool(6);
    cm_net::pool_server server(56000, &thread_pool, request_handler,
        request_dealloc);

    CPPUNIT_ASSERT( server.is_started() == true );


    // try again by restarting the client thread...
    client.start();
    CPPUNIT_ASSERT( client.is_started() && !client.is_done());

    // wait for all the thread to finish working
    while( !client.is_done() ) {
        timespec delay = {0, 10000000};   // 10 ms
        nanosleep(&delay, NULL);
    }

     // wait for pool_server threads to complete all work tasks
    thread_pool.wait_all();
    
   clock_gettime(CLOCK_REALTIME, &now);
   double total = cm_time::duration(start, now);

   cm_log::info(cm_util::format("total time: %7.4lf secs", total));
}