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

class unit_thread: public cm_thread::basic_thread {

    int count = 0;

    bool process() {

        // if count reached, end the thread (return false)
        if(count++ > 100)
            return false;

        cm_log::info(cm_util::format("count = [%d]", count));
        
        // signal continue (return true)
        return true;
    }  
};


void threadTest::test_thread() {

    cm_log::file_logger log("./log/thread_test.log");
    set_default_logger(&log);

    unit_thread thread;

    thread.start();

    CPPUNIT_ASSERT( thread.is_done() == false );

    sleep(2);

    CPPUNIT_ASSERT( thread.is_done() == true );

}

