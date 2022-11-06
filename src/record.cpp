/*
 * Copyright (c) 2022, Tom Oleson <tom dot oleson at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * The names of its contributors may NOT be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "log.h"
#include "xml_reader.h"
#include "record.h"

const string cm_record::field::to_string() {
    return cm_util::format("field: name=[%s], type=[%s], length=[%u]", name.c_str(), type.c_str(), length);
}

const string cm_record::record_spec::to_string() {
    return cm_util::format("record_spec: name=[%s], version=[%s], delimiter=[%s], size=[%u]", name.c_str(), version.c_str(), delimiter.c_str(), size());
}

const string cm_record::record::to_string() {
    string output;
    vector<string>::iterator p = data.begin();
    for(int index = 0; p != data.end(); index++) {
        output.append( cm_util::format("%d=[%s]", index, p->c_str()) );
        if(++p != data.end()) output.append(",");
    }
    return cm_util::format("record: spec=[%s:%s] data=[%s]", record_spec->get_name().c_str(), record_spec->get_version().c_str(), output.c_str());
}

bool cm_record::xml_load_record_spec(const string spec, const string name, const string version, cm_record::record_spec *spec_ptr) {

    spec_ptr->set_name(name);
    spec_ptr->set_version(version);

	try {
        // spec can be an in-memory string starting with "<?xml" or the path to an xml file...
		cm_xml::XMLReader reader(spec);

		if(reader.Read() && reader.IsStartElement("spec")) {

			while(reader.Read()) {

				// process a record element...
				// <record name="person" version="1.0" delimiter="|" >
				if(reader.IsStartElement("record")) {

                    if(reader.HasAttributes()) {

                        string spec_name = reader.GetAttribute("name");
                        string spec_version = reader.GetAttribute("version");
                        string spec_delimiter = reader.GetAttribute("delimiter");

                        if(spec_name == name && spec_version == version) {

                            spec_ptr->set_delimiter(spec_delimiter);

                            // process field elements...
                            // <field name="record_type" type="string" length="0" />
                            while(reader.Read() && reader.IsStartElement("field")) {
                                if(reader.HasAttributes()) {
                                    string name = reader.GetAttribute("name");
                                    string type = reader.GetAttribute("type");

                                    string length = reader.GetAttribute("length");
                                    size_t len = (size_t) atoi(length.c_str());

                                    cm_record::field field(name, type, len);
                                    spec_ptr->add_field(field);
                                }
                            } // field element
                        }
					}
				} // record element

			}
        }
	} catch(const string &s) {
		    cm_log::error(s);
			return false;
	}
	catch(...) {
            string s = cm_util::format("Exception: strerror=[%s]", strerror(errno));
            cm_log::error(s);
            return false;
    }
    return true;
}
