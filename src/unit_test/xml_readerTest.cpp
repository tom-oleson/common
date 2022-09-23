
#include <cppunit/config/SourcePrefix.h>
#include <errno.h>

#include "xml_readerTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( xml_readerTest );


void xml_readerTest::setUp() { }


string xml_readerTest::read_xml(const string spec) {
	try {
		XMLReader reader(spec);

		if(reader.Read() && reader.IsStartElement("spec")) {

			while(reader.Read()) {

				// process a properties element...
				// <properties name="MDR_AAA" >
				if(reader.IsStartElement("properties")) {
					// process property elements...
					// <property name="somename" value="somevalue" />
					while(reader.Read() && reader.IsStartElement("property")) { 

							if(reader.HasAttributes()) {
								int count = reader.GetAttributeCount();
								for(int index = 0; index < count; index++) {
									reader.MoveToAttribute(index);
								}
							}
					}

				}
				// process a record element...
				// <record id="1" type="01" name="REQUEST" >
				else if(reader.IsStartElement("record")) {
						// process field elements...
						// <field name="record_type" type="string" pos="0" />
						while(reader.Read() && reader.IsStartElement("field")) {
							if(reader.HasAttributes()) {
								int count = reader.GetAttributeCount();
								for(int index = 0; index < count; index++) {
									reader.MoveToAttribute(index);
								}
							}

						}
					}
				}
			}

	} catch(const string &s) {
		    cm_log::error(s);
			return (s);
	}
	catch(...) {
            string s = cm_util::format("Exception: strerror=[%s]", strerror(errno));
            cm_log::error(s);
            return (s);
    }

	return "OK";
}

void xml_readerTest::test_read_file() {

		    cm_log::file_logger log("./log/xml_reader_test.log");
			log.set_log_level(cm_log::level::trace);
    		set_default_logger(&log);
			

			string result = read_xml("stat_spec1.00.00.xml");
			CPPUNIT_ASSERT(result == "OK");
}

void xml_readerTest::test_read_memory() {

		    cm_log::file_logger log("./log/xml_reader_test.log");
			log.set_log_level(cm_log::level::trace);
    		set_default_logger(&log);


			string xml ="<?xml version='1.0'?>" \
					"<spec>" \
						"<record id='20' type='20' name='ALERT'>" \
							"<field name='timestamp' type='string' pos='0'/>" \
							"<field name='hostname' type='string' pos='1'/>" \
						"</record>" \
						"<properties name='MDR_AAA'>" \
							"<property name='executable' value='*'/>" \
							"<property name='symptomatic' value='true'/>" \
						"</properties>" \
					"</spec>";

			string result = read_xml(xml);
			CPPUNIT_ASSERT(result == "OK");
}

