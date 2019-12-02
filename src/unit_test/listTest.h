/**********************************************************************
*
* listTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_LIST_TEST_H
#define CPP_UNIT_LIST_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "list.h"
#include "log.h" 

using namespace std;

class listTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( listTest );
    CPPUNIT_TEST( test_list );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_list();
};


#endif
