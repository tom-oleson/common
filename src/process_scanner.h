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

#ifndef __PROCESS_SCANNER_H
#define __PROCESS_SCANNER_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define __LINUX_BOX__
#ifndef __LINUX_BOX__
#include <procfs.h>
#endif

#include <sys/types.h>
#include <dirent.h>

#include "log.h"

namespace cm {

struct ps_process {

	ps_process() {}
	ps_process(pid_t _pid, const string &_name) : name(_name), pid(_pid) { }
	ps_process(const ps_process &pr) : name(pr.name), pid(pr.pid) { } 

	ps_process &operator = (const ps_process &pr) {
		name.assign(pr.name);
		pid = pr.pid;	
		return *this;
	}

	const string to_string() {
		string ss;
		return cm::format(ss, "ps_process: pid=[%d], name=[%s]", pid, name.c_str());
	}

	void dump() {
		cm_log:::trace(to_string());
	}

	string name;
	pid_t pid;

private:

};

class process_scanner  {

	vector<ps_process *> match_list;	// list of matched processes

	pid_t process_id;
	string name;

    int scan_name(pid_t pid, string &name);
    int scan_one(pid_t pid, string &name);
    int scan_all(const string &name);

public:
	process_scanner(pid_t pid);
	process_scanner(string &_name);
	process_scanner(const char *_name);

	~process_scanner();
	void free_match_list();

	bool is_process_running(pid_t pid);
	pid_t get_process_id() { return process_id; }
	string get_name() { return name; }

	// return a list of processes that matched in scan_all()
	vector<ps_process *> &get_match_list() { return match_list; }
};

}


#endif
