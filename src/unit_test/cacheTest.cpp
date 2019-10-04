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
        return true;
    }

    bool do_read(const std::string &name) {
        cm_log::info(cm_util::format("read: %s", name.c_str()));

        std::string value = cm_store::mem_store.find(name);
        if(value.size() > 0) {
            cm_log::info(cm_util::format("%s", value.c_str()));
        }
        else {
            cm_log::info("NF");
        }
        return true;
    }

    bool do_remove(const std::string &name) {
        cm_log::info(cm_util::format("remove: %s", name.c_str()));

        int num = cm_store::mem_store.remove(name);
        cm_log::info(cm_util::format("(%d)", num));
        return true;
    }

    bool do_watch(const std::string &name, const std::string &tag) {
        cm_log::info(cm_util::format("watch: %s %s", name.c_str(), tag.c_str()));

        std::string value = cm_store::mem_store.find(name);
        cm_log::info(cm_util::format("%s:%s", tag.c_str(), value.c_str()));
        return true;
    }

    bool do_error(const std::string &expr, const std::string &err) {
        cm_log::error(cm_util::format("%s: %s", err.c_str(), expr.c_str()));
        return false;
    }
};

void cacheTest::test_cache() {

    cm_log::file_logger server_log("./log/cache_test.log");
    set_default_logger(&server_log);

    unit_processor processor;
    cm_cache::cache cache(&processor);

    cache.eval("+foo 'bar'");   // add
    cache.eval("$foo");         // read
    cache.eval("*foo #0");      // watch
    cache.eval("-foo");         // remove
    cache.eval("$foo");         // read
    
    cache.eval("+name 'Tom'");
    cache.eval("$name");
    cache.eval("-name");

    cache.eval("+bin 'x:deadbeef'");
    cache.eval("$bin");
    CPPUNIT_ASSERT( cm_store::mem_store.check("bin") == true );

    cache.eval("-bin");
    CPPUNIT_ASSERT( cm_store::mem_store.check("bin") == false );

    cache.eval("-bin");

    cache.eval("+object {name = 'Tom', id = 369}");
    cache.eval("$object");

    std::string encoded = base64_encode("A string to encode.");
    cache.eval( cm_util::format("+encoded '%s'", encoded.c_str()) );
    cache.eval("$encoded");

    cache.eval( cm_util::format("+%s 'http://somedomain.com'", encoded.c_str()));
    cache.eval( cm_util::format("$%s", encoded.c_str()));
    
    cache.eval("+bad1");
    cache.eval("-{bad2}");
    cache.eval("*bad3 #");
    cache.eval("$+bad4");
}

