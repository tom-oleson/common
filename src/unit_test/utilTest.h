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


using namespace std;
//using namespace cm;

class utilTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( utilTest );
    CPPUNIT_TEST( test_bin2hex );
    CPPUNIT_TEST( test_format_local_timestamp );
    CPPUNIT_TEST( test_format_utc_timestamp );
    CPPUNIT_TEST( get_timezone_offset );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
  void test_bin2hex();
  void test_format_local_timestamp();
  void test_format_utc_timestamp();
  void get_timezone_offset();
  
};


#endif
