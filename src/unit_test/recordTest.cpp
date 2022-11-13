
#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout
#include <cstdint>	// for uint32_t
#include <arpa/inet.h> 	// for htonl()

#include "recordTest.h"

#include "record.h"
#include "log.h"
#include "timewatcher.h"
#include "store.h" 

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

    cm_record::record task(&task_spec);

    // add some data

    string timestamp = cm_time::clock_gmt_timestamp();

    task.set("id", timestamp);
    task.set("description", "Create new software");
    task.set("priority", "1");
    task.set("activity", "Software Development");
    task.set("notes", "Includes design, code and test cycles.");
    task.set("tags", "#software #development");
    task.set("start", timestamp);
    task.set("status", " ");

    cm_log::info(task.to_string());

    CPPUNIT_ASSERT(task.get("id") == timestamp);
    CPPUNIT_ASSERT(task.get("description") == "Create new software");
    CPPUNIT_ASSERT(task.get("priority") == "1");
    CPPUNIT_ASSERT(task.get("activity") == "Software Development");
    CPPUNIT_ASSERT(task.get("notes") == "Includes design, code and test cycles.");
    CPPUNIT_ASSERT(task.get("tags") == "#software #development");
    CPPUNIT_ASSERT(task.get("start") == timestamp);
    CPPUNIT_ASSERT(task.get("due") == "");
    CPPUNIT_ASSERT(task.get("done") == "");
    CPPUNIT_ASSERT(task.get("status") == " ");

}

void recordTest::test_format_and_parse() {

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

    cm_record::record task(&task_spec);

    // add some data

    string timestamp = cm_time::clock_gmt_timestamp();

    task.set("id", timestamp);
    task.set("description", "Create new software");
    task.set("priority", "1");
    task.set("activity", "Software Development");
    task.set("notes", "Includes design, code and test cycles.");
    task.set("tags", "#software #development");
    task.set("start", timestamp);
    task.set("status", " ");

    // format the record
    string output;
    int out_count = task.format(output);
    
    cm_log::info(output);

    // clear the data
    task.clear();

    // parse the output back into the record object

    int in_count = task.parse(output);

    CPPUNIT_ASSERT(in_count == out_count);

    CPPUNIT_ASSERT(task.get("id") == timestamp);
    CPPUNIT_ASSERT(task.get("description") == "Create new software");
    CPPUNIT_ASSERT(task.get("priority") == "1");
    CPPUNIT_ASSERT(task.get("activity") == "Software Development");
    CPPUNIT_ASSERT(task.get("notes") == "Includes design, code and test cycles.");
    CPPUNIT_ASSERT(task.get("tags") == "#software #development");
    CPPUNIT_ASSERT(task.get("start") == timestamp);
    CPPUNIT_ASSERT(task.get("due") == "");
    CPPUNIT_ASSERT(task.get("done") == "");
    CPPUNIT_ASSERT(task.get("status") == " ");

}


// global
cm_store::info_store<std::string,std::string> priority_store;


void recordTest::test_lookup() {

    cm_log::file_logger log("./log/record_test.log");
    set_default_logger(&log);

    // load record spec
    string xml = "<?xml version='1.0'?>" \
        "<spec>" \
            "<record name='priority' version='1.0' delimiter='|'>" \
                "<field name='code' type='string' length='1'/>" \
                "<field name='description' type='string'/>" \
            "</record>" \
        "</spec>";
    cm_record::record_spec priority_spec;
    bool ret = xml_load_record_spec(xml, "priority", "1.0", &priority_spec);

    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(priority_spec.get_name() == "priority");
    CPPUNIT_ASSERT(priority_spec.get_version() == "1.0");

    CPPUNIT_ASSERT(priority_spec.get_index("code") == 0);
    CPPUNIT_ASSERT(priority_spec.get_index("description") == 1);


    // parse records and place in store
    cm_record::record priority(&priority_spec);
    string records =    "1|Urgent/Important\n" \
                        "2|Urgent/Not Important\n" \
                        "3|Not Urgent/Important\n" \
                        "4|Not Urgent/Not Important";

    // use input streingstream to read records from string...
    istringstream is(records);

    // note you can use ifstream to get records from a file...
    // ifstream is("/path/to/file");

    ret = cm_record::load_records_to_store(is, priority, "code", "description", &priority_store);

    CPPUNIT_ASSERT(priority_store.find("1") == "Urgent/Important");
    CPPUNIT_ASSERT(priority_store.find("2") == "Urgent/Not Important");
    CPPUNIT_ASSERT(priority_store.find("3") == "Not Urgent/Important");
    CPPUNIT_ASSERT(priority_store.find("4") == "Not Urgent/Not Important");
}

// global
cm_store::info_store<std::string,std::string> status_store;

void recordTest::test_lookup_from_file() {

    cm_log::file_logger log("./log/record_test.log");
    set_default_logger(&log);

    // load record spec
    string xml = "status-1.0-spec.xml";
    cm_record::record_spec status_spec;
    bool ret = xml_load_record_spec(xml, "status", "1.0", &status_spec);

    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(status_spec.get_name() == "status");
    CPPUNIT_ASSERT(status_spec.get_version() == "1.0");

    CPPUNIT_ASSERT(status_spec.get_index("code") == 0);
    CPPUNIT_ASSERT(status_spec.get_index("description") == 1);

    // parse records and place in store
    cm_record::record status(&status_spec);
    ifstream is("./status-1.0-map.txt");
    CPPUNIT_ASSERT(is.is_open() == true);

    ret = cm_record::load_records_to_store(is, status, "code", "description", &status_store);

    CPPUNIT_ASSERT(status_store.find(" ") == "Open");
    CPPUNIT_ASSERT(status_store.find("X") == "Done");
    CPPUNIT_ASSERT(status_store.find("D") == "Delegate");
    CPPUNIT_ASSERT(status_store.find("I") == "Ignore");

}
