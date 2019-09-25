/**********************************************************************
*
* threadTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "threadTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( threadTest );

//void threadTest::setUp() { }
//void threadTest::tearDown() { }

struct unit_thread: public cm_thread::basic_thread {

    int count = 0;

    bool process() {

        for(int x = 0; x < 10000; x++) {
            if(++count % 10000 == 0)
                cm_log::info(cm_util::format("count = [%d]", count));
        } 
        return count < 1000000;
    }  
};

void threadTest::test_thread() {

    cm_log::file_logger log("./log/thread_test.log");
    set_default_logger(&log);

    unit_thread thread;

    thread.start();

    CPPUNIT_ASSERT( thread.is_done() == false );

    sleep(2);

    CPPUNIT_ASSERT( thread.count == 1000000 );
    CPPUNIT_ASSERT( thread.is_done() == true );

}


////////////////////////// thread pool /////////////////////////////


void do_work(void *arg) {

    (void) arg;

    int count = 0;
    char buf[128] = { '\0' };
    size_t sz = 0;

    for(int n = 0; n < 1000000; ++n) {
        if(++count % 10000 == 0)
            sz = snprintf(buf, sizeof(buf), "count = [%d]", count);
    }

    //cm_log::info(std::string(buf, sz));
}

void threadTest::test_thread_pool() {

    cm_log::file_logger log("./log/pool_test.log");
    log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");
    set_default_logger(&log);

    cm_thread::pool thread_pool(4);
    CPPUNIT_ASSERT( thread_pool.thread_count() == 4);


    for(int n = 0; n < 1000; ++n) {
        thread_pool.add_task(do_work, nullptr);
    }
    // wait for threads to complete all work tasks
    thread_pool.wait_all();
    thread_pool.log_counts();


    // another set of work
    for(int n = 0; n < 500; ++n) {
        thread_pool.add_task(do_work, nullptr);
    }
    // wait for threads to complete all work tasks
    thread_pool.wait_all();
    thread_pool.log_counts();


    // all the work is done
    CPPUNIT_ASSERT( thread_pool.work_queue_count() == 0 );
}