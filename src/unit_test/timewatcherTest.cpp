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


void timewatcherTest::test_clock_time() {

	time_t seconds = cm_time::clock_seconds();
	// make sure timewatcher has a non-zero time
	CPPUNIT_ASSERT( seconds > 0 );

	time_t millis = 0xdeadbeef;	//insane value
	time_t nanos = 0xdeadbeef;	// insane value
	seconds = cm_time::clock_seconds(&millis, &nanos);

	// make sure millis is now sane
	CPPUNIT_ASSERT( millis >= 0);
	CPPUNIT_ASSERT( millis <= 999); 

	// make sure nanos is now sane
	CPPUNIT_ASSERT( nanos >= 0);
	CPPUNIT_ASSERT( nanos <= 999999999);

	// make sure timewatcher thread sees time change
	sleep(2);
	CPPUNIT_ASSERT( cm_time::clock_seconds() >= seconds+1 );
}

void timewatcherTest::test_total_millis() {

    timespec res = cm_time::clock_res();
	timespec now = cm_time::clock_time();
	time_t seconds = cm_time::seconds(now);
	time_t millis = cm_time::millis(now);
	time_t total_millis = cm_time::total_millis(now);
	time_t computed_total_millis = ((seconds * 1000L) + millis);

        CPPUNIT_ASSERT( total_millis == computed_total_millis );
}

void timewatcherTest::test_duration() {

    timespec delay = {3, 500000000};   // 3.5 seconds

    timespec start = cm_time::clock_time();
    nanosleep(&delay, NULL);
    timespec finish = cm_time::clock_time();
 
    double diff = cm_time::duration(start, finish);

    CPPUNIT_ASSERT( diff > 3.5f && diff < 3.6f );
}

