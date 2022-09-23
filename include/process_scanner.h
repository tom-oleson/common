/*
 * Copyright (C)2019, Tom Oleson <tom dot oleson at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are me
 *
 *   * Redistributions of source code must retain the above copyright notice
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

#ifndef __PROCESS_SCANNER_H
#define __PROCESS_SCANNER_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <memory>

#ifndef __LINUX_BOX__
#include <procfs.h>
#endif

#include <sys/types.h>
#include <dirent.h>

#include "log.h"

namespace cm_process {

struct ps_process {

	ps_process(pid_t _pid, const std::string &_name) : name(_name), pid(_pid) { }
	ps_process(const ps_process &pr) : name(pr.name), pid(pr.pid) { } 

	void dump() {
		cm_log::info(cm_util::format("ps_process: pid=[%d], name=[%s]", pid, name.c_str()));
	}

	std::string name;
	pid_t pid;

private:

};

class process_scanner  {

	// list of matched processes in scan_all()
	std::vector<std::unique_ptr<ps_process>> match_list;

	pid_t process_id;
	std::string name;

    int scan_name(pid_t pid, std::string &name);
    int scan_one(pid_t pid, std::string &name);
    int scan_all(const std::string &name);

public:
	process_scanner(pid_t pid);
	process_scanner(std::string &_name);
	process_scanner(const char *_name);

	bool is_process_running(pid_t pid);
	pid_t get_process_id() { return process_id; }
	std::string get_name() { return name; }

	// return a list of processes that matched in scan_all()
	std::vector<std::unique_ptr<ps_process>> &get_match_list() { return match_list; }
};

}

#endif
