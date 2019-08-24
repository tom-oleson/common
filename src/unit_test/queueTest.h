/**********************************************************************
*
* queueTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_STORE_TEST_H
#define CPP_UNIT_STORE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "queue.h"
#include "log.h" 


using namespace std;

class queueTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( queueTest );
    CPPUNIT_TEST( test_memory_queue );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_memory_queue();
};


#endif
