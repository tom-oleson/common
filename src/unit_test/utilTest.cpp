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


void utilTest::test_format() {

    std::string s;
    cm_util::format(s, "%02d %02d %02d %03d %03d", 5, 4, 3, 2, 1);
    CPPUNIT_ASSERT( s == "05 04 03 002 001");
}


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

void utilTest::test_file_stat() {

    size_t size = 0;
    time_t seconds = 0;
    int ret = cm_util::file_stat("run_tests", &size, &seconds); 
    
    CPPUNIT_ASSERT( ret == 0 && size > 0 && seconds > 0);
}

void utilTest::test_next_midnight() {

    time_t seconds = cm_time::clock_seconds();
    time_t next_midnight = cm_util::next_midnight(seconds);

    CPPUNIT_ASSERT( next_midnight > 0 );

}

void utilTest::test_next_hour() {

    struct tm local_tm;
    time_t seconds = cm_time::clock_seconds();
    time_t next_hour = cm_util::next_hour(seconds, 1);

    CPPUNIT_ASSERT( next_hour > 0 );

    next_hour = cm_util::prev_midnight(seconds);
    next_hour = cm_util::next_hour(next_hour, 3);   // 3am
    next_hour = cm_util::next_hour(next_hour, 3);   // 6am
    next_hour = cm_util::next_hour(next_hour, 3);   // 9am
    next_hour = cm_util::next_hour(next_hour, 3);   // 12pm


    next_hour = cm_util::prev_midnight(seconds);
    next_hour = cm_util::next_hour(next_hour, 24);  // next midnight
}

void utilTest::test_next_calendar_time() {

    time_t seconds = cm_time::clock_seconds();
    time_t next_time = cm_util::next_calendar_time(seconds, 13, 0, 0);   // next 1pm

    CPPUNIT_ASSERT( next_time > 0 );
}

void utilTest::test_calendar_time() {

    struct tm local_tm;
    time_t seconds = cm_time::clock_seconds();
    time_t cal_time = cm_util::calendar_time(seconds, local_tm);

    CPPUNIT_ASSERT( cal_time == seconds );
}

void utilTest::test_format_filename_timestamp() {

    time_t seconds = 0;
    std::string result = cm_util::format_filename_timestamp(seconds, true /*gmt*/);

    CPPUNIT_ASSERT( result == "19700101_000000" );
}

void utilTest::test_remove() {

    bool appended = cm_util::append_to_file("remove_test.log", "This file should be removed.");

    int removed = cm_util::remove("remove_test.log");

    CPPUNIT_ASSERT( appended == true && removed == 0);
}

void utilTest::test_rename() {
     bool appended = cm_util::append_to_file("rename_test.log", "This renamed file should be removed.");

    int renamed = cm_util::rename("rename_test.log", "remove_test.log");

    int removed = cm_util::remove("remove_test.log");

    CPPUNIT_ASSERT( appended == true && renamed == 0 && removed == 0);

}
