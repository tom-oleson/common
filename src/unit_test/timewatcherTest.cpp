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
	CPPUNIT_ASSERT( seconds > 0 );
}

void timewatcherTest::test_timeTotalMillis() {

	timespec now = readTime();
	time_t seconds = timeSeconds(now);
	time_t millis = timeMillis(now);
	int64_t total_millis = timeTotalMillis(now);

        CPPUNIT_ASSERT( total_millis == ((seconds * 1000) + (millis > 0LL ? millis:0LL) ));
}

