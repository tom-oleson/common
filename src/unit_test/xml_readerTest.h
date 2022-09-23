

#ifndef CPP_UNIT_XMLREADERTEST_H
#define CPP_UNIT_XMLREADERTEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include "log.h"
#include "xml_reader.h"

using namespace std;
using namespace cm_xml;

class xml_readerTest : public CPPUNIT_NS::TestFixture {

  CPPUNIT_TEST_SUITE( xml_readerTest );
  CPPUNIT_TEST( test_read_file );
  CPPUNIT_TEST( test_read_memory );
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp();

protected:
  void test_read_file();
  void test_read_memory();

  string read_xml(const string spec);
};


#endif
