
#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "recordTest.h"

#include "record.h"
#include "log.h"
#include "timewatcher.h" 

CPPUNIT_TEST_SUITE_REGISTRATION( recordTest );

//void recordTest::setUp() { }
//void recordTest::tearDown() { }

void recordTest::test_record_spec() {

    cm_log::file_logger log("./log/record_test.log");
    set_default_logger(&log);

    cm_record::field name("name", "string", 0);
    cm_record::field age("age", "int", 0);

    CPPUNIT_ASSERT( name.get_name() == "name");
    CPPUNIT_ASSERT( age.get_name() == "age");

    cm_record::record_spec person_spec("person", "1.0");

    person_spec.add_field(name);
    person_spec.add_field(age);

    CPPUNIT_ASSERT(person_spec.get_name() == "person");
    CPPUNIT_ASSERT(person_spec.get_version() == "1.0");

    cm_log::info(person_spec.to_string());

    for(int index = 0; index < person_spec.size(); index++) {
        cm_log::info(person_spec.get_field(index).to_string());
    }
}

void recordTest::test_load_record_spec() {

    cm_log::file_logger log("./log/record_test.log");
    set_default_logger(&log);

    string xml = "<?xml version='1.0'?>" \
        "<spec>" \
            "<record name='task' version='1.0' delimiter='|'>" \
                "<field name='id' type='timestamp' length='14'/>" \
                "<field name='description' type='string'/>" \
                "<field name='priority' type='int' length='1'/>" \
                "<field name='activity' type='string'/>" \
                "<field name='notes' type='string'/>" \
                "<field name='tags' type='string'/>" \
                "<field name='start' type='timestamp' length='14'/>" \
                "<field name='due' type='timestamp' length='14'/>" \
                "<field name='done' type='timestamp' length='14'/>" \
                "<field name='status' type='string' length='1'/>" \
            "</record>" \
        "</spec>";

    cm_record::record_spec spec;
    bool ret = xml_load_record_spec(xml, "task", "1.0", &spec);

    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(spec.get_name() == "task");
    CPPUNIT_ASSERT(spec.get_version() == "1.0");

    cm_log::info(spec.to_string());

    for(int index = 0; index < spec.size(); index++) {
        cm_log::info(spec.get_field(index).to_string());
    }
}

void recordTest::test_record() {

    cm_log::file_logger log("./log/record_test.log");
    set_default_logger(&log);

    string xml = "<?xml version='1.0'?>" \
        "<spec>" \
            "<record name='task' version='1.0' delimiter='|'>" \
                "<field name='id' type='timestamp' length='14'/>" \
                "<field name='description' type='string'/>" \
                "<field name='priority' type='int' length='1'/>" \
                "<field name='activity' type='string'/>" \
                "<field name='notes' type='string'/>" \
                "<field name='tags' type='string'/>" \
                "<field name='start' type='timestamp' length='14'/>" \
                "<field name='due' type='timestamp' length='14'/>" \
                "<field name='done' type='timestamp' length='14'/>" \
                "<field name='status' type='string' length='1'/>" \
            "</record>" \
        "</spec>";

    cm_record::record_spec task_spec;
    bool ret = xml_load_record_spec(xml, "task", "1.0", &task_spec);

    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(task_spec.get_name() == "task");
    CPPUNIT_ASSERT(task_spec.get_version() == "1.0");

    // create record object

    cm_record::record task(task_spec);

    // add some data

    string timestamp = cm_time::clock_gmt_timestamp();

    task.set_data("id", timestamp);
    task.set_data("description", "Create new software");
    task.set_data("priority", "1");
    task.set_data("activity", "Software Development");
    task.set_data("notes", "Includes desigh, code and test cycles.");
    task.set_data("tags", "#software #development");
    task.set_data("start", timestamp);
    task.set_data("status", " ");

    cm_log::info(task.to_string());

    CPPUNIT_ASSERT(task.get_data("id") == timestamp);
    CPPUNIT_ASSERT(task.get_data("description") == "Create new software");
    CPPUNIT_ASSERT(task.get_data("priority") == "1");
    CPPUNIT_ASSERT(task.get_data("activity") == "Software Development");
    CPPUNIT_ASSERT(task.get_data("notes") == "Includes desigh, code and test cycles.");
    CPPUNIT_ASSERT(task.get_data("tags") == "#software #development");
    CPPUNIT_ASSERT(task.get_data("start") == timestamp);
    CPPUNIT_ASSERT(task.get_data("due") == "");
    CPPUNIT_ASSERT(task.get_data("done") == "");
    CPPUNIT_ASSERT(task.get_data("status") == " ");

}


