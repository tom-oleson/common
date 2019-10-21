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
    bool do_add(const std::string &name, const std::string &value, cm_cache::cache_event &event) {
        cm_log::info(cm_util::format("add: %s %s", name.c_str(), value.c_str()));

        cm_store::mem_store.set(name, value);
        event.name.assign(name);
        event.value.assign(value);
        event.result.assign("OK");
        return do_result(event);
    }

    bool do_read(const std::string &name, cm_cache::cache_event &event) {
        cm_log::info(cm_util::format("read: %s", name.c_str()));

        event.value = cm_store::mem_store.find(name);
        event.name.assign(name);
        if(event.value.size() > 0) {
            event.result.assign(cm_util::format("%s", event.value.c_str()));
        }
        else {
            event.result.assign("NF");
        }
        return do_result(event);
    }

    bool do_read_remove(const std::string &name, cm_cache::cache_event &event) {
        cm_log::info(cm_util::format("read_remove: %s", name.c_str()));

        event.value = cm_store::mem_store.find(name);
        event.name.assign(name);
        if(event.value.size() > 0) {
            event.result.assign(cm_util::format("%s", event.value.c_str()));
            cm_store::mem_store.remove(name);
        }
        else {
            event.result.assign("NF");
        }
        return do_result(event);
    }

    bool do_remove(const std::string &name, cm_cache::cache_event &event) {
        cm_log::info(cm_util::format("remove: %s", event.name.c_str()));

        int num = cm_store::mem_store.remove(name);
        event.name = name;
        event.result.assign(cm_util::format("(%d)", num));
        return do_result(event);
    }

    bool do_watch(const std::string &name, const std::string &tag, cm_cache::cache_event &event) {
        cm_util::format("watch: %s %s", event.name.c_str(), event.tag.c_str());

        event.value = cm_store::mem_store.find(name);
        event.name = name;
        event.result.assign(cm_util::format("%s:%s", tag.c_str(), event.value.c_str()));
        return do_result(event);
    }

    bool do_watch_remove(const std::string &name, const std::string &tag, cm_cache::cache_event &event) {
        cm_util::format("watch: %s %s", event.name.c_str(), event.tag.c_str());

        event.value = cm_store::mem_store.find(name);
        event.name = name;
        event.result.assign(cm_util::format("%s:%s", tag.c_str(), event.value.c_str()));
        int num = cm_store::mem_store.remove(name);

        return do_result(event);
    }

    bool do_result(cm_cache::cache_event &event) {
        cm_log::info(cm_util::format("%s", event.result.c_str()));
        return true;
    }

    bool do_input(const std::string &in_str, cm_cache::cache_event &event) { 
        event.request.assign(in_str);
        return true;
    }

    bool do_error(const std::string &expr, const std::string &err, cm_cache::cache_event &event) {
        event.result.assign(cm_util::format("%s: %s", err.c_str(), expr.c_str()));
        cm_log::error(event.result);
        return false;
    }
};

void cacheTest::test_cache() {

    cm_log::file_logger server_log("./log/cache_test.log");
    set_default_logger(&server_log);

    unit_processor processor;
    cm_cache::cache cache(&processor);
    cm_cache::cache_event event;

    cache.eval("+foo 'bar'", event);   // add
    cache.eval("$foo", event);         // read
    cache.eval("*foo #0", event);      // watch
    cache.eval("-foo", event);         // remove
    cache.eval("$foo", event);         // read
    
    cache.eval("+name 'Tom'", event);
    cache.eval("$name", event);
    cache.eval("-name", event);

    cache.eval("+age 56", event);
    cache.eval("$age", event);

    cache.eval("+bin 'x:deadbeef'", event);
    cache.eval("$bin", event);
    CPPUNIT_ASSERT( cm_store::mem_store.check("bin") == true );

    cache.eval("-bin", event);
    CPPUNIT_ASSERT( cm_store::mem_store.check("bin") == false );

    cache.eval("-bin", event);

    cache.eval("+object {name = 'Tom', id = 369}", event);
    cache.eval("$object", event);

    std::string encoded = base64_encode("A string to encode.");
    cache.eval( cm_util::format("+encoded '%s'", encoded.c_str()), event);
    cache.eval("$encoded", event);

    cache.eval( cm_util::format("+%s 'http://somedomain.com'", encoded.c_str()), event);
    cache.eval( cm_util::format("$%s", encoded.c_str()), event);
    
    cache.eval("+bad1", event);
    cache.eval("-{bad2}", event);
    cache.eval("*bad3 #", event);
    cache.eval("$+bad4", event);
}
