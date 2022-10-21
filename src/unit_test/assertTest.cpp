
#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "assertTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( assertTest );

cm_log::file_logger assert_log("./log/assert_test.log");


void assertTest::setUp() {
    assert_log.set_log_level(cm_log::level::info);
    set_default_logger(&assert_log);
}
//void assertTest::tearDown() { }


bool true_func() { return true; }
bool false_func() { return false; }


void assertTest::test_assert() {

    CM_ASSERT(0 == 0);
    CM_ASSERT(0 > 1);

    CM_ASSERT(true_func() == false);
    CM_ASSERT(false_func() == true);


    CPPUNIT_ASSERT( CM_UNIT_TEST_EXEC(true_func()) == true );
    CPPUNIT_ASSERT( CM_UNIT_TEST_EXEC(false_func()) == false );

}
