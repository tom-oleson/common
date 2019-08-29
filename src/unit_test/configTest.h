/**********************************************************************
*
* configTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_CONFIG_TEST_H
#define CPP_UNIT_CONFIG_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "config.h"
#include "log.h" 


using namespace std;

class configTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( configTest );
    CPPUNIT_TEST( test_memory_config );
    CPPUNIT_TEST( test_file_config );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_memory_config();
    void test_file_config();
};


#endif
