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



void utilTest::test_base64() {

    std::string s1 = "abc";
    std::string e1 = base64_encode(s1);
    CPPUNIT_ASSERT( e1 == "YWJj" );
    std::string d1 = base64_decode(e1);
    CPPUNIT_ASSERT( d1 == s1 );

    std::string s2 = "abcd";
    std::string e2 = base64_encode(s2);
    CPPUNIT_ASSERT( e2 == "YWJjZA==" );
    std::string d2 = base64_decode(e2);
    CPPUNIT_ASSERT( d2 == s2 );

    std::string s3 = "abcde";
    std::string e3 = base64_encode(s3);
    CPPUNIT_ASSERT( e3 == "YWJjZGU=");
    std::string d3 = base64_decode(e3);
    CPPUNIT_ASSERT( d3 == s3 );

}

void utilTest::test_format() {

    std::string s = cm_util::format("%02d %02d %02d %03d %03d", 5, 4, 3, 2, 1);
    CPPUNIT_ASSERT( s == "05 04 03 002 001");
}

void utilTest::test_format_str() {

    std::string s;
    cm_util::format_string(s,"%02d %02d %02d %03d %03d", 5, 4, 3, 2, 1);
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

        // test lowercase output
        len = cm_util::bin2hex((const unsigned char *) bin, 8, out, sizeof(out), true);
        CPPUNIT_ASSERT( string(out) == "deadbeefdeadbeef" );	
}

void utilTest::test_bin2hex_line() {

    cm_log::file_logger log("./log/bin2hex_line.log");
    set_default_logger(&log);

    char buf[512] = { '\0'};
    const char *str = "Hello, world!";

    cm_util::bin2hex_line(buf, sizeof(buf), str, strlen(str), 16, cm_util::hex_lower);
    cm_log::info(buf);

    CPPUNIT_ASSERT( std::string(buf) == "48 65 6c 6c 6f 2c 20 77 6f 72 6c 64 21            Hello, world!");

    // test through the default logger
    str = "Now is the time for all good men to come to the aid of their country.";
    cm_log::hex_dump(cm_log::level::info, str, strlen(str)+1, 16);

    // test through logger reference
    str = "Now is the time for all good men to come to the aid of their country, again and again.";
    log.hex_dump(cm_log::level::info, str, strlen(str)+1, 16);
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

void utilTest::test_dir_scan() {

    cm_log::file_logger log("./log/test_dir_scan.log");
    log.set_log_level(cm_log::level::info);
    set_default_logger(&log);

    std::vector<std::string> matches;

    // scan ./log directory and match any that end with ".log"
    // note: use of raw string literal to avoid need to escape \ in regex
    cm_util::dir_scan("./log", R"(.+\.log$)", matches);

    std::vector<std::string>::iterator p = matches.begin();
    while(p != matches.end()) {
        std::string name = *p;
        cm_log::info(cm_util::format("%s", name.c_str() ));
        p++;
    }
    cm_log::info(cm_util::format("matched: %d", matches.size()));

    CPPUNIT_ASSERT( matches.size() > 0 );
}

void utilTest::test_strlcpy() {

    char buf[10+1] = { '\0' };

    // copy string that will fit
    CPPUNIT_ASSERT( cm_util::strlcpy(buf, "0123456789", sizeof(buf)) == 10);

    // copy string that will NOT fit
    CPPUNIT_ASSERT( cm_util::strlcpy(buf, "0123456789012345", sizeof(buf)) == 10);


}

void utilTest::test_split() {

    std::string s = "one+two+three+four+five";

    std::vector<std::string> v = cm_util::split (s, '+');

    CPPUNIT_ASSERT( v[0] == "one");
    CPPUNIT_ASSERT( v[1] == "two");
    CPPUNIT_ASSERT( v[2] == "three");
    CPPUNIT_ASSERT( v[3] == "four");
    CPPUNIT_ASSERT( v[4] == "five");

}


void utilTest::test_regex_match() {

    // note: use raw string literal to avoid need to escape \ and quotes 
    // if they are in your regex: example: R"(.+\.log$)"

    CPPUNIT_ASSERT(cm_util::regex_match("tom oleson", "^tom.+son$") == 0);
    CPPUNIT_ASSERT(cm_util::regex_match("arduino001", "^arduino[0-9][0-9][0-9]") == 0);
}


void utilTest::test_regex_replace() {

    cm_log::file_logger log("./log/regex_replace.log");
    log.set_log_level(cm_log::level::info);
    set_default_logger(&log);

    // note: use raw string literal to avoid need to escape \ and quotes 
    // if they are in your regex: example: R"(.+\.log$)"

    std::string s = "arduino001 {count:5133}";
    
    CPPUNIT_ASSERT(cm_util::regex_replace(s, R"(\{count)", "{time:1572897267.566,count") == 0);
    CPPUNIT_ASSERT( s == "arduino001 {time:1572897267.566,count:5133}");

    cm_log::info(cm_util::format("result = %s", s.c_str()));


}

void utilTest::test_JS_macro() {

    cm_log::file_logger log("./log/JS_macro.log");
    log.set_log_level(cm_log::level::info);
    set_default_logger(&log);

    // note: use raw string literal to avoid need to escape \ and quotes 
    // if they are in your regex: example: R"(.+\.log$)"

    std::string s = "arduino001 {"JS("count")":5133}";
    cm_log::info(cm_util::format("result = %s", s.c_str()));


}

