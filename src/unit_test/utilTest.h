/**********************************************************************
*
*       %name           $Id: CommonTest.h 16453 2015-04-14 19:48:34Z g701023 $
*       %version        $Rev: 16453 $
*       %created_by     $Author: g701023 $
*       %date           $Date: 2015-04-14 15:48:34 -0400 (Tue, 14 Apr 2015) $
*
**********************************************************************/


#ifndef CPP_UNIT_UTIL_TEST_H
#define CPP_UNIT_UTIL_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>


using namespace std;
//using namespace cm;

class utilTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( utilTest );
    CPPUNIT_TEST( test_bin2hex );
  CPPUNIT_TEST_SUITE_END();

public:
  //void setUp();
  //void tearDown();

protected:
  void test_bin2hex();
  
};


#endif
