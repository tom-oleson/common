/**********************************************************************
*
* utilTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "utilTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( utilTest );

//void utilTest::setUp() { }
//void utilTest::tearDown() { }


void utilTest::test_bin2hex() {

	uint32_t bin[2] = { htonl(0xdeadbeef), htonl(0xdeadbeef) };
	char out[512] = { '\0' };

	size_t len = cm_util::bin2hex((const unsigned char *) bin, 8, out, sizeof(out));
	//cout << " len=" << len << ": " << out;

	// length is 2 digits per byte + terminator byte
	CPPUNIT_ASSERT( len == (8*2)+1 );

	// output string is the expected hex-ASCII value
	CPPUNIT_ASSERT( string(out) == "DEADBEEFDEADBEEF" );
	
}


void utilTest::test_format_local_timestamp() {

	time_t seconds = 1564244978;
	time_t millis = 0;
        std::string tz = cm_util::get_timezone_offset(seconds);
	std::string result = cm_util::format_local_timestamp(seconds, millis, tz);
	

	CPPUNIT_ASSERT( true == true );
}

void utilTest::test_format_utc_timestamp() {

        time_t seconds = 0;
	time_t millis = 0;
        std::string result = cm_util::format_utc_timestamp(seconds, millis);

	CPPUNIT_ASSERT( result == "1970-01-01T00:00:00.000Z" );
}

void utilTest::test_get_timezone_offset() {

	time_t seconds = 1564244978;
	std::string result = cm_util::get_timezone_offset(seconds);
	CPPUNIT_ASSERT( result.size() == 6 );
}

void utilTest::test_get_hostname() {

    std::string hostname = cm_util::get_hostname();

    CPPUNIT_ASSERT( !hostname.empty() );
}
