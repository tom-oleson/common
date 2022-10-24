
#ifndef CPP_UNIT_RECORD_TEST_H
#define CPP_UNIT_RECORD_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "record.h"
#include "log.h" 


using namespace std;

class recordTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( recordTest );
    CPPUNIT_TEST( test_record );
    CPPUNIT_TEST( test_load_record_spec );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
    void test_record();
    void test_load_record_spec();
};


#endif
