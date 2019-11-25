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

void a_split(const char *s) {

    char my_buf[80] = {'\0' };
    char *bp = my_buf;
    const char *cp = s;

    for(char *sp = bp; ;cp++ ) {

        if(*cp == '\0' || *cp == '\n') {
            cm_log::info(sp);
            sp = bp+1;
        }
        if(*cp == '\0') break;
        *bp++ = *cp;
    }
}

void bufferTest::test_buffer() {

    cm_log::file_logger log("./log/buffer.log");
    log.set_log_level(cm_log::level::info);
    set_default_logger(&log);

    std::string s;



    a_split("one");
    a_split("1\n2\n\n3");
    a_split("one\ntwo\nthree\nfour");


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

    
    char sbuf[32];
    cm_buf::buffer sout(sbuf, sizeof(sbuf));
    sout.clear();
    sout.append("c-string");
    sout.append(":test");
    cm_log::hex_dump(cm_log::level::info, sbuf, sizeof(sbuf), 16);
    s.assign(sbuf, strlen(sbuf));
    CPPUNIT_ASSERT(s == "c-string:test");





}
