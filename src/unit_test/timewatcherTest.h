/**********************************************************************
*
* timewatcherTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_TIMEWATCHER_TEST_H
#define CPP_UNIT_TIMEWATCHER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <timewatcher.h>


using namespace std;
//using namespace cm;

class timewatcherTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( timewatcherTest );
    CPPUNIT_TEST( test_clock_time );
    CPPUNIT_TEST( test_total_millis );
    CPPUNIT_TEST( test_duration );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
  void test_clock_time();
  void test_total_millis();
  void test_duration();
};


#endif
