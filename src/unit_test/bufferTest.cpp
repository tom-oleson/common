/**********************************************************************
*
* bufferTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "bufferTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( bufferTest );

//void bufferTest::setUp() { }
//void bufferTest::tearDown() { }

void bufferTest::test_buffer() {

    cm_log::file_logger log("./log/buffer.log");
    log.set_log_level(cm_log::level::info);
    set_default_logger(&log);

    std::string s;

    char buf[10] = { '\0' };
    memset(buf, 0xFF, sizeof(buf));

    cm_buf::buffer out(buf, 5);


    out.append('A');
    cm_log::hex_dump(cm_log::level::info, buf, sizeof(buf), 16);

    out.append("Hello", 6);
    cm_log::hex_dump(cm_log::level::info, buf, sizeof(buf), 16);
    s.assign(buf, strlen(buf));
    CPPUNIT_ASSERT(s == "AHel");

    int i = 10000;
    out.clear();
    out.append(i);
    cm_log::hex_dump(cm_log::level::info, buf, sizeof(buf), 16);
    s.assign(buf, strlen(buf));
    CPPUNIT_ASSERT(s == "1000");

    long l = 10000L;
    out.clear();
    out.append(l);
    cm_log::hex_dump(cm_log::level::info, buf, sizeof(buf), 16);
    s.assign(buf, strlen(buf));
    CPPUNIT_ASSERT(s == "1000");    

    float f = 10.25;
    out.clear();
    out.append(f);
    cm_log::hex_dump(cm_log::level::info, buf, sizeof(buf), 16);
    s.assign(buf, strlen(buf));
    CPPUNIT_ASSERT(s == "10.2");




}
