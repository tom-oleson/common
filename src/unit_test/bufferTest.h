/**********************************************************************
*
* bufferTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_BUFFER_TEST_H
#define CPP_UNIT_BUFFER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "buffer.h"
#include "log.h" 

using namespace std;

class bufferTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( bufferTest );
    CPPUNIT_TEST( test_buffer );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_buffer();
};


#endif
