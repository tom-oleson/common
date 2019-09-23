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

struct task_data {

    int count = 0;
    char buf[128] = { '\0' };
    size_t buf_sz = sizeof(buf);
    size_t sz = 0;

};

void do_work(void *data) {

    task_data *p = (task_data *)data;

    int &count = p->count;
    char *buf = p->buf;
    size_t &buf_sz = p->buf_sz;
    size_t &sz = p->sz;

    for(int n = 0; n < 1000000; ++n) {
        if(++count % 10000 == 0)
            sz = snprintf(buf, buf_sz, "count = [%d]", count);
    } 
    cm_log::info(std::string(buf, sz));
}

void threadTest::test_thread_pool() {

    cm_log::file_logger log("./log/thread_pool_test.log");
    set_default_logger(&log);

    cm_thread::pool thread_pool(10);
    CPPUNIT_ASSERT( thread_pool.thread_count() == 10);

    std::vector<task_data *> data_list;

    for(int n = 0; n < 100; ++n) {
        task_data *p = new task_data;
        data_list.push_back(p);
        thread_pool.add_task(do_work, p);
    }

    int count = 0;
    while(thread_pool.work_queue_count() > 0 && ++count < 15000) {
        timespec delay = {0, 1000000};   // 1 ms
        nanosleep(&delay, NULL);
    }

    for(auto it = data_list.begin(); it < data_list.end(); ) {
        delete (*it);
        it = data_list.erase(it);
    }

}