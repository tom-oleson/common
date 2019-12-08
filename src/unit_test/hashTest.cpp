/**********************************************************************
*
* hashTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout

#include "hashTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( hashTest );

//void hashTest::setUp() { }
//void hashTest::tearDown() { }


void hashTest::test_md5_hash() {

    cm_log::file_logger log("./log/test_md5_hash.log");
    set_default_logger(&log);

    std::string s("thinkpad-t43");
    unsigned char hash[16];
    char out[40] = { '\0' };

    // compute hash for string
    cm_hash::md5_hash(s.c_str(), s.size(), hash);

    // convert to hex-ASCII
    cm_util::bin2hex((const unsigned char *) hash, sizeof(hash), out, sizeof(out));

    cm_log::info(out);

    CPPUNIT_ASSERT( std::string(out) == "292294A62A5AC406267378837C581D5B" );

}

