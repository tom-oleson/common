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


#ifndef __XMLREADER_H
#define __XMLREADER_H

#define LIBXML_READER_ENABLED

#include <string>
#include <libxml2/libxml/xmlreader.h>

using namespace std;

namespace cm_xml {

class XMLReader{
	string xml_file;
	xmlTextReaderPtr reader;
	const xmlChar *name, *value;

	string cur_name;
	string cur_value;
	string cur_attr;
	

	XMLReader();
	
	
public:
    XMLReader(const string &xml_file);
    ~XMLReader();
    
    bool Read();
    int GetNodeType();

    
    const string &GetName();
    const string &GetAttribute(const string &name);
    const string &GetAttribute(int index);
    int GetAttributeCount();
    bool MoveToAttribute(int index);
    const string &GetValue();
    
    bool IsStartElement(const string &name);
    bool HasAttributes();
    bool IsEndElement(const string &name);
    bool IsEmptyElement();
    bool IsStartElement();
    bool IsEndElement();
    bool IsText();
	bool IsAttribute();


};
} // namespace cm_xml
#endif
