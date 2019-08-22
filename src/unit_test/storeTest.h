/**********************************************************************
*
* storeTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_STORE_TEST_H
#define CPP_UNIT_STORE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "store.h"
#include "log.h" 


using namespace std;

class storeTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( storeTest );
    CPPUNIT_TEST( test_memory_store );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_memory_store();
};


#endif
