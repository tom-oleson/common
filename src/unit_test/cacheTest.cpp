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
    bool do_add(const std::string &name, const std::string &value, std::string &result) {
        cm_log::info(cm_util::format("add: %s %s", name.c_str(), value.c_str()));

        cm_store::mem_store.set(name, value);
        do_result("OK");
        return true;
    }

    bool do_read(const std::string &name, std::string &result) {
        cm_log::info(cm_util::format("read: %s", name.c_str()));

        std::string value = cm_store::mem_store.find(name);
        if(value.size() > 0) {
            do_result(cm_util::format("%s", value.c_str()));
        }
        else {
            do_result("NF");
        }
        return true;
    }

    bool do_remove(const std::string &name, std::string &result) {
        cm_log::info(cm_util::format("remove: %s", name.c_str()));

        int num = cm_store::mem_store.remove(name);
        do_result(cm_util::format("(%d)", num));
        return true;
    }

    bool do_watch(const std::string &name, const std::string &tag, std::string &result) {
        cm_util::format("watch: %s %s", name.c_str(), tag.c_str());

        std::string value = cm_store::mem_store.find(name);
        do_result(cm_util::format("%s:%s", tag.c_str(), value.c_str()));
        return true;
    }

    bool do_result(const std::string &result) {
        cm_log::info(cm_util::format("%s", result.c_str()));
        return false;
    }

    bool do_input(const std::string &in_str, std::string &expr) { 
        return true;
    }

    bool do_error(const std::string &expr, const std::string &err, std::string &result) {
        cm_log::error(cm_util::format("%s: %s", err.c_str(), expr.c_str()));
        return false;
    }
};

void cacheTest::test_cache() {

    cm_log::file_logger server_log("./log/cache_test.log");
    set_default_logger(&server_log);

    unit_processor processor;
    cm_cache::cache cache(&processor);
    std::string result;

    cache.eval("+foo 'bar'", result);   // add
    cache.eval("$foo", result);         // read
    cache.eval("*foo #0", result);      // watch
    cache.eval("-foo", result);         // remove
    cache.eval("$foo", result);         // read
    
    cache.eval("+name 'Tom'", result);
    cache.eval("$name", result);
    cache.eval("-name", result);

    cache.eval("+age 56", result);
    cache.eval("$age", result);

    cache.eval("+bin 'x:deadbeef'", result);
    cache.eval("$bin", result);
    CPPUNIT_ASSERT( cm_store::mem_store.check("bin") == true );

    cache.eval("-bin", result);
    CPPUNIT_ASSERT( cm_store::mem_store.check("bin") == false );

    cache.eval("-bin", result);

    cache.eval("+object {name = 'Tom', id = 369}", result);
    cache.eval("$object", result);

    std::string encoded = base64_encode("A string to encode.");
    cache.eval( cm_util::format("+encoded '%s'", encoded.c_str()), result);
    cache.eval("$encoded", result);

    cache.eval( cm_util::format("+%s 'http://somedomain.com'", encoded.c_str()), result);
    cache.eval( cm_util::format("$%s", encoded.c_str()), result);
    
    cache.eval("+bad1", result);
    cache.eval("-{bad2}", result);
    cache.eval("*bad3 #", result);
    cache.eval("$+bad4", result);
}

