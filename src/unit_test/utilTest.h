/**********************************************************************
*
* utilTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_UTIL_TEST_H
#define CPP_UNIT_UTIL_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "util.h"
#include "log.h" 
#include "timewatcher.h"
#include "base64.h"


using namespace std;

class utilTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( utilTest );
    CPPUNIT_TEST( test_base64 );
    CPPUNIT_TEST( test_format_str );
    CPPUNIT_TEST( test_format );
    CPPUNIT_TEST( test_bin2hex );
    CPPUNIT_TEST( test_bin2hex_line);
    CPPUNIT_TEST( test_format_local_timestamp );
    CPPUNIT_TEST( test_format_utc_timestamp );
    CPPUNIT_TEST( test_format_filename_timestamp );
    CPPUNIT_TEST( test_get_timezone_offset );
    CPPUNIT_TEST( test_get_hostname );
    CPPUNIT_TEST( test_dir_scan );
    CPPUNIT_TEST( test_file_stat );
    CPPUNIT_TEST( test_remove );
    CPPUNIT_TEST( test_rename );
    CPPUNIT_TEST( test_calendar_time );
    CPPUNIT_TEST( test_next_midnight );
    CPPUNIT_TEST( test_next_hour );
    CPPUNIT_TEST( test_next_calendar_time );
    CPPUNIT_TEST( test_strlcpy );
    CPPUNIT_TEST( test_split );
    CPPUNIT_TEST( test_regex_match );
    CPPUNIT_TEST( test_regex_replace );
    CPPUNIT_TEST( test_JS_macro );
    CPPUNIT_TEST( test_extract );

  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_base64();
    void test_format_str();
    void test_format();
    void test_bin2hex();
    void test_bin2hex_line();
    void test_format_local_timestamp();
    void test_format_utc_timestamp();
    void test_get_timezone_offset();
    void test_get_hostname();
    void test_file_stat();
    void test_format_filename_timestamp();
    void test_remove();
    void test_rename();
    void test_calendar_time();
    void test_next_midnight();
    void test_next_hour();
    void test_next_calendar_time();
    void test_dir_scan();
    void test_strlcpy();
    void test_split();
    void test_regex_match();
    void test_regex_replace();
    void test_JS_macro();
    void test_extract();
};


#endif
