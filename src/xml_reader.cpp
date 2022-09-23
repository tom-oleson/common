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

#include <string.h>

#include "log.h"
#include "xml_reader.h"

namespace cm_xml {

XMLReader::XMLReader() { }

XMLReader::XMLReader(const string &spec) {
    string s;
    reader = NULL;
    
    // see if it is an in-memory xml string...
    if(spec.substr(0,5) == "<?xml") {

     cm_log::info("Creating XMLReader for in-memory xml string");

		// try using it as an in-memory XML string
		reader = xmlReaderForMemory	(
			 spec.c_str(),
			 strlen(spec.c_str()),
			 "memory.xml",
			 NULL,
			 XML_PARSE_NOBLANKS |	// remove blank nodes
			 XML_PARSE_NOCDATA	// merge CDATA as text nodes
			);
		if(NULL == reader) {
			s = "Call to xmlReaderForMemory() failed";
		    cm_log::error(s);
			throw s;
		 }
	}
    else {
    	xml_file = spec;
        cm_log::info(cm_util::format("Creating XMLReader for file: [%s]", xml_file.c_str()));

        reader = xmlReaderForFile(xml_file.c_str(),
         NULL,
    	 XML_PARSE_NOBLANKS |	// remove blank nodes
    	 XML_PARSE_NOCDATA	// merge CDATA as text nodes
    	 );

    	if(NULL == reader) {
    		s = "Call to xmlReaderForFile() failed";
    	    cm_log::error(s);
    		throw s;
    	}
    }

    cm_log::info("XMLReader created.");
}
 
XMLReader::~XMLReader() {
	if(reader != NULL) {
        xmlFreeTextReader(reader);
    }
	xmlCleanupParser();
    CM_LOG_TRACE cm_log::trace("XMLReader destroyed.");
}

bool XMLReader::Read() {
	//string s;
	// 1: node parsed, 0: no node parse and no error, !=0: parse error 
	int ret = xmlTextReaderRead(reader);
	if(ret < 0) {
		throw string("xmlTextReaderRead: parse error");
	}

	if(ret == 1) {
		int node_type = GetNodeType();
		const string &node_name = GetName();
		if(IsText()) {
			const string &node_value = GetValue();
		    CM_LOG_TRACE cm_log::trace( cm_util::format("name=[%s], node_type=[%d], value=[%s]", node_name.c_str(), node_type, node_value.c_str()));
		}
		else CM_LOG_TRACE cm_log::trace( cm_util::format("name=[%s], node_type=[%d]", node_name.c_str(), node_type));
	}

	return (1 == ret);
}

const string &XMLReader::GetName() {
	cur_name.assign("");
	const xmlChar *node_name = xmlTextReaderConstName(reader);
	if(NULL != node_name) {
		cur_name.assign((const char *) node_name);
	}
	return cur_name;
}

const string &XMLReader::GetAttribute(const string &_name) {
	string s;
	cur_attr.assign("");
	xmlChar *attr_value = xmlTextReaderGetAttribute(reader, (const xmlChar *) _name.c_str());
	if(NULL != attr_value) {
		cur_attr.assign((const char *) attr_value);
		free(attr_value);	// caller must free allocated memory...	
	    CM_LOG_TRACE cm_log::trace( cm_util::format("attribute: name=[%s], value=[%s]", _name.c_str(), cur_attr.c_str()));
	}
	return cur_attr;
}

bool XMLReader::MoveToAttribute(int index) {
	string s;
        int ret = xmlTextReaderMoveToAttributeNo(reader, index);

        if(ret == 1) {
                int node_type = GetNodeType();
                const string &node_name = GetName();
                if(IsText() || IsAttribute()) {
                        const string &node_value = GetValue();
                        CM_LOG_TRACE cm_log::trace( cm_util::format("name=[%s], node_type=[%d], value=[%s]", node_name.c_str(), node_type, node_value.c_str()));
                }
                else CM_LOG_TRACE cm_log::trace( cm_util::format("name=[%s], node_type=[%d]", node_name.c_str(), node_type));
        }

        return (1 == ret);
}

    //xmlTextReaderAttributeCount

const string &XMLReader::GetAttribute(int index) {
	cur_attr.assign("");
	xmlChar *attr_value = xmlTextReaderGetAttributeNo(reader, index);
	if(NULL != attr_value) {
		cur_attr.assign((const char *) attr_value);
		free(attr_value);	// caller must free allocated memory...	
	}
	return cur_attr;
}

int XMLReader::GetAttributeCount() {
	int count = xmlTextReaderAttributeCount(reader);
	return count;
}
const string &XMLReader::GetValue() {
	cur_value.assign("");
	// get value of node (memory is freed on the next call to xmlTextReaderRead)...
	value = xmlTextReaderConstValue(reader);
	if(NULL != value) {
		cur_value.assign((const char *) value);
	}
	return cur_value;
}

bool XMLReader::IsStartElement(const string &_name) {
	const char *name = (const char *) xmlTextReaderConstName(reader);
	bool ret = strlen(name) == _name.size() &&
		strcmp(name, _name.c_str()) == 0 &&
		IsStartElement();
	return ret;
	
}
bool XMLReader::IsEndElement(const string &_name) {
	const char *name = (const char *) xmlTextReaderConstName(reader);
	bool ret = strlen(name) == _name.size() &&
			strcmp(name, _name.c_str()) == 0 &&
			IsEndElement();
	return ret;
	
}

bool XMLReader::IsEmptyElement() {
	int ret = xmlTextReaderIsEmptyElement(reader);
	return (1 == ret);
}

bool XMLReader::IsStartElement() {
	return (XML_READER_TYPE_ELEMENT == GetNodeType());
}

bool XMLReader::IsEndElement() {
	return (XML_READER_TYPE_END_ELEMENT == GetNodeType());
}

bool XMLReader::IsText() {
	return (XML_READER_TYPE_TEXT == GetNodeType());
}

bool XMLReader::IsAttribute() {
	return (XML_READER_TYPE_ATTRIBUTE == GetNodeType());
}

int XMLReader::GetNodeType() {
	int node_type =	xmlTextReaderNodeType(reader);
	return node_type;
}

bool XMLReader::HasAttributes() {
	int ret = xmlTextReaderHasAttributes(reader);
	return (1 == ret);
}

}

