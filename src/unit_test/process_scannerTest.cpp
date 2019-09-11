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

#include <cppunit/config/SourcePrefix.h>

#include <vector>
#include "process_scannerTest.h"
#include "timewatcher.h"

CPPUNIT_TEST_SUITE_REGISTRATION( process_scannerTest );


cm_log::file_logger *ps_log;

void process_scannerTest::setUp() { 

    // create a file logger
    cm_log::file_logger *ps_log = new cm_log::file_logger("./log/process_scanner.log");
    //ps_log->set_log_level(cm_log::level::trace);

    // set it as the default logger to be used by package macros
    set_default_logger(ps_log);
}

void process_scannerTest::tearDown() {
    delete ps_log;
}

void process_scannerTest::test_constructors() {

	std::string name = "run_tests";

	pid_t pid = getpid();
	cm_log::info(cm_util::format("pid = [%d]", pid));

	cm_process::process_scanner pid_ps(pid);
	cm_log::info(cm_util::format("pid_ps.name = [%s]", pid_ps.get_name().c_str()));
	cm_log::info(cm_util::format("pid_ps.pid = [%d]", pid_ps.get_process_id()));

	CPPUNIT_ASSERT( pid_ps.get_name() == name );
	CPPUNIT_ASSERT( pid_ps.get_process_id() == pid);

	cm_process::process_scanner name_ps(name);
	cm_log::info(cm_util::format("name_ps.name = [%s]", name_ps.get_name().c_str()));
	cm_log::info(cm_util::format("name_ps.pid = [%d]", name_ps.get_process_id()));

	CPPUNIT_ASSERT( name_ps.get_name() == name);  
	CPPUNIT_ASSERT( name_ps.get_process_id() == pid);
	CPPUNIT_ASSERT( name_ps.is_process_running(pid) );
}

void process_scannerTest::test_multi() {
    std::string name = "watchdog";

    cm_process::process_scanner name_ps(name);
    cm_log::info(cm_util::format("name_ps.name = [%s]", name_ps.get_name().c_str()));
    cm_log::info(cm_util::format("name_ps.pid = [%d]", name_ps.get_process_id()));

    CPPUNIT_ASSERT( name_ps.get_name() == name);	//
    CPPUNIT_ASSERT( name_ps.get_process_id() > 0);

    // dump all processes in match list...
    for(auto &ps: name_ps.get_match_list()) {
        ps->dump();
    }
}

