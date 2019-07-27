/**********************************************************************
*
* utilTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "utilTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( utilTest );

//void utilTest::setUp() { }
//void utilTest::tearDown() { }


void utilTest::test_bin2hex() {

	uint32_t bin[2] = { htonl(0xdeadbeef), htonl(0xdeadbeef) };
	char out[512] = { '\0' };

	size_t len = bin2hex((const unsigned char *) bin, 8, out, sizeof(out));
	//cout << " len=" << len << ": " << out;

	// length is 2 digits per byte + terminator byte
	CPPUNIT_ASSERT( len == (8*2)+1 );

	// output string is the expected hex-ASCII value
	CPPUNIT_ASSERT( string(out) == "DEADBEEFDEADBEEF" );
	
}
