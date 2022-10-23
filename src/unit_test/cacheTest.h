
#ifndef CPP_UNIT_STORE_TEST_H
#define CPP_UNIT_STORE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "cache.h"
#include "log.h" 
#include "base64.h"

using namespace std;

class cacheTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( cacheTest );
    CPPUNIT_TEST( test_cache );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_cache();
};


#endif
