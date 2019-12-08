/**********************************************************************
*
* hashTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_HASH_TEST_H
#define CPP_UNIT_HASH_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <util.h>
#include "log.h" 
#include "hash.h"


using namespace std;

class hashTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( hashTest );
    CPPUNIT_TEST( test_md5_hash );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_md5_hash();
};


#endif
