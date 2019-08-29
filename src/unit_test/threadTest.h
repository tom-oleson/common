/**********************************************************************
*
* threadTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_THREAD_TEST_H
#define CPP_UNIT_THREAD_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "thread.h"
#include "log.h" 


using namespace std;

class threadTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( threadTest );
    CPPUNIT_TEST( test_thread );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_thread();
};


#endif
