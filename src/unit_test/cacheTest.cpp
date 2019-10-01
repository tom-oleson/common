/**********************************************************************
*
* cacheTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "cacheTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( cacheTest );

//void cacheTest::setUp() { }
//void cacheTest::tearDown() { }


class unit_processor: public cm_cache::scanner_processor {

public:
    bool do_add(const std::string &name, const std::string &value) {
        cm_log::info(cm_util::format("add: %s %s", name.c_str(), value.c_str()));

        cm_store::mem_store.set(name, value);
    }

    bool do_read(const std::string &name) {
        cm_log::info(cm_util::format("read: %s", name.c_str()));

        std::string value = cm_store::mem_store.find(name);
        cm_log::info(cm_util::format("%s", value.c_str()));
    }

    bool do_remove(const std::string &name) {
        cm_log::info(cm_util::format("remove: %s", name.c_str()));

        int num = cm_store::mem_store.remove(name);
        cm_log::info(cm_util::format("%d", num));
    }

    bool do_watch(const std::string &name, const std::string &tag) {
        cm_log::info(cm_util::format("watch: %s %s", name.c_str(), tag.c_str()));

        std::string value = cm_store::mem_store.find(name);
        cm_log::info(cm_util::format("%s:%s", tag.c_str(), value.c_str()));

    }
};


void cacheTest::test_cache() {

    cm_log::file_logger server_log("./log/cache_test.log");
    set_default_logger(&server_log);
    //server_log.set_message_format("${date_time}${millis} [${lvl}] <${thread}>: ${msg}");

    unit_processor processor;
    cm_cache::cache cache(&processor);

    cache.eval("+foo bar");     // add
    cache.eval("$foo");         // read
    cache.eval("*foo #0");      // watch
    cache.eval("-foo");         // remove
    cache.eval("$foo");         // read
    

    cache.eval("+name 'Tom Oleson'");
    cache.eval("$name");

    CPPUNIT_ASSERT( true );
}

