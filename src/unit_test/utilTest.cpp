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

#include "log.h"
#include "mutex.h"

CPPUNIT_TEST_SUITE_REGISTRATION( utilTest );


//void utilTest::setUp() { }
//void utilTest::tearDown() { }


void utilTest::test_bin2hex() {

	cm::mutex mutex;

	mutex.lock();
	mutex.unlock();

	cm_log::info("This is the first log message!");

	uint32_t bin[2] = { htonl(0xdeadbeef), htonl(0xdeadbeef) };
	char out[512] = { '\0' };

	size_t len = bin2hex((const unsigned char *) bin, 8, out, sizeof(out));
	//cout << " len=" << len << ": " << out;

	// length is 2 digits per byte + terminator byte
	CPPUNIT_ASSERT( len == (8*2)+1 );

	// output string is the expected hex-ASCII value
	CPPUNIT_ASSERT( string(out) == "DEADBEEFDEADBEEF" );
	
}
