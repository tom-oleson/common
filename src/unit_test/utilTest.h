/**********************************************************************
*
* utilTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_UTIL_TEST_H
#define CPP_UNIT_UTIL_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>


using namespace std;
//using namespace cm;

class utilTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( utilTest );
    CPPUNIT_TEST( test_bin2hex );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
  void test_bin2hex();
  
};


#endif
