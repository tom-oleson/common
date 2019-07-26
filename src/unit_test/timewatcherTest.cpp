/**********************************************************************
*
* timewatcherTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "timewatcherTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( timewatcherTest );

//void timewatcherTest::setUp() { }
//void timewatcherTest::tearDown() { }


void timewatcherTest::test_getTime() {

	time_t seconds = getTime();
	// make sure timewatcher has a non-zero time
	CPPUNIT_ASSERT( seconds > 0 );

	time_t millis = 0xdeadbeef;	//insane value
	time_t nanos = 0xdeadbeef;	// insane value
	seconds = getTime(&millis, &nanos);

	// make sure millis is now sane
	CPPUNIT_ASSERT( millis >= 0);
	CPPUNIT_ASSERT( millis <= 999); 

	// make sure nanos is now sane
	CPPUNIT_ASSERT( nanos >= 0);
	CPPUNIT_ASSERT( nanos <= 999999999);

	// make sure timewatcher thread sees time change
	sleep(2);
	CPPUNIT_ASSERT( getTime() >= seconds+1 );
}

void timewatcherTest::test_timeTotalMillis() {

	timespec now = readTime();
	time_t seconds = timeSeconds(now);
	time_t millis = timeMillis(now);
	time_t total_millis = timeTotalMillis(now);
	time_t computed_total_millis = ((seconds * 1000L) + millis);

        CPPUNIT_ASSERT( total_millis == computed_total_millis );
}

