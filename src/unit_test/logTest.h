/**********************************************************************
*
* logTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_LOG_TEST_H
#define CPP_UNIT_LOG_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>

#include "util.h"
#include "log.h"


using namespace std;
//using namespace cm;

class logTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( logTest );
    CPPUNIT_TEST( test_default_logger );
    CPPUNIT_TEST( test_file_logger );
    CPPUNIT_TEST( test_format_log_timestamp );
    CPPUNIT_TEST( test_format_millis );
    CPPUNIT_TEST( test_log_level_if_macros );
    CPPUNIT_TEST( test_get_part_index );
    CPPUNIT_TEST( test_parse_message_format );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_default_logger();
    void test_file_logger();
    void test_format_log_timestamp();
    void test_format_millis();
    void test_log_level_if_macros();  
    void test_get_part_index();
    void test_parse_message_format(); 
};


#endif
