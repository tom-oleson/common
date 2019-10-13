/**********************************************************************
*
* networkTest.h
*
**********************************************************************/


#ifndef CPP_UNIT_NETWORK_TEST_H
#define CPP_UNIT_NETWORK_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "network.h"
#include "log.h" 


using namespace std;

class networkTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( networkTest );
    CPPUNIT_TEST( test_client_connect );
    CPPUNIT_TEST( test_network );
    CPPUNIT_TEST( test_network_thread_pool );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_client_connect();
    void test_network();
    void test_network_thread_pool();
};


#endif
