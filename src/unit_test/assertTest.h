/**********************************************************************
*
* cacheTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_STORE_TEST_H
#define CPP_UNIT_STORE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "assert.h"
#include "log.h" 

using namespace std;

class assertTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( assertTest );
    CPPUNIT_TEST( test_assert );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  //void tearDown();

protected:
    void test_assert();
};


#endif
