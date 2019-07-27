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
    CPPUNIT_TEST( test_file_logger );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
  void test_file_logger();
  
};


#endif
