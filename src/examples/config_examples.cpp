/*
 * Copyright (C)2019, Tom Oleson <tom dot oleson at gmail dot com>
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
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
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

#include "util.h"
#include "log.h"
#include "config.h"


void memory_config_example() {

    std::string s;

    cm_config::set("host", cm_util::get_hostname());

    std::string host = cm_config::get("host");


    bool found = cm_config::check("host");
    if(found) {
        cm_log::info(cm_util::format(s, "host: [%s]", host.c_str()));
    }

    // default value when key is not present
    std::string some_option = cm_config::get("some_option", "none" );
    cm_log::info(cm_util::format(s, "some_option: [%s]", some_option.c_str())); 

}


void file_config_example() {

    std::string s;
    std::string path = "./example_config.cfg";

    cm_config::file_config config_file(path);
    config_file.load();
    set_default_config(&config_file);

    // access base level option
    std::string option = cm_config::get("option", "none");

    // access section option
    std::string section_option = cm_config::get("section.option", "none");

    // access nested sub-section option
    std::string subsection_option = cm_config::get("section.subsection.option", "none");
   
    cm_log::info(cm_util::format(s, "1: [%s], 2: [%s], 3: [%s]", 
        option.c_str(), section_option.c_str(), subsection_option.c_str())); 

}
