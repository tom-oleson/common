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

#include <iostream>
#include <fstream>

#include "process_scanner.h"


// We know the process id but not the name
cm_process::process_scanner::process_scanner(pid_t pid) : name(""), process_id(pid) {
	scan_name(process_id, name);
}

// We know the name but not the process id
cm_process::process_scanner::process_scanner(std::string &_name) : name(_name), process_id(0) {
	scan_all(name.c_str());
	
}
cm_process::process_scanner::process_scanner(const char *_name) : name(_name), process_id(0) {
	scan_all(name.c_str());
}

/*
cm_process::process_scanner::~process_scanner() {
	free_match_list();
}

void cm_process::process_scanner::free_match_list() {

    for(auto &ps: match_list) {
    	delete ps;
    }

    match_list.clear();
}
*/

bool cm_process::process_scanner::is_process_running(pid_t pid) {

	return process_id == pid && scan_name(process_id, name) > 0;
}

#ifdef __LINUX_BOX__
int cm_process::process_scanner::scan_name(pid_t pid, std::string &_name) {

	  std::string pathname;
	  std::fstream fs;
	  char pr_fname[256] = {'\0'};

	  cm_util::format_string(pathname, "/proc/%d/status", (int)pid);

	  fs.open(pathname.c_str(), std::fstream::in);
	  if (!fs.is_open()) {
	        cm_log::error( cm_util::format("%s open failed: %s", pathname.c_str(), strerror(errno)));
		return -1;
	  }

	  fs.getline(pr_fname, sizeof(pr_fname));

	  if (fs.bad()) {
	        cm_log::error( cm_util::format("%s getline failed: %s", pathname.c_str(), strerror(errno)));
		fs.close();
	    	return -2;
	  }
	  fs.close();
	  // file: proc/[pid]/status
	  // First line has the name and offset 6 is where the name value starts
	  // Name:\t<name>
	  _name.assign(pr_fname + 6);

	  return 1;

}

#else
// Sun Solaris method
int cm_process::process_scanner::scan_name(pid_t pid, std::string &_name) {

  std::string pathname;
  int fd;
  psinfo_t  psinfo;

  cm_util::format(pathname, "/proc/%d/psinfo", (int)pid);

  if ((fd = open(pathname.c_str(), O_RDONLY)) < 0) {
        cm_log::error( cm_util::format("%s open failed: %s", pathname.c_str(), strerror(errno)));
        return -1;
  }

  if (read(fd, &psinfo, sizeof (psinfo)) != sizeof (psinfo)) {
        cm_log::error(  cm_util::format("%s read failed", pathname.c_str(), strerror(errno)));
        (void) close(fd);
        return -2;
  }

  (void) close(fd);

  _name.assign(psinfo.pr_fname);

  return 1;
}
#endif


int cm_process::process_scanner::scan_one(pid_t pid, std::string &_name) {

  int result = scan_name(pid, _name);
  if(result < 0) return result;

  CM_LOG_TRACE { cm_log::trace( cm_util::format("process_name=[%s]", _name.c_str())); }
  if(0 == strncmp(name.c_str(), _name.c_str(), name.length())) {
  cm_log::info( cm_util::format("found: %s, pid=[%d]", _name.c_str(), (int)pid));
	process_id = pid;
	return 1;
  }

  return 0;
}

int cm_process::process_scanner::scan_all(const std::string &_name) {
  pid_t pid = 1;
  DIR *dp;
  struct dirent *d;

  if (NULL == (dp = opendir("/proc"))) {
    cm_log::error( cm_util::format("/proc opendir failed: %s", strerror(errno)));
	return -2;
  }

  int match = 0;
  while (NULL != (d = readdir(dp))) {
    CM_LOG_TRACE { cm_log::trace( cm_util::format("dir=[%s]", d->d_name)); }
	pid = atoi(d->d_name);

	if(pid > 0) {
	 CM_LOG_TRACE { cm_log::trace( cm_util::format("pid=[%d]", (int)pid)); }

		std::string find_name(_name);
		//match = scan_one(pid, _find_name);
		// stop scanning on match (keep scanning on open/read error)
		//if(1 == match) break; 

		// capture matching process...
		
		if(scan_one(pid, find_name) > 0) {
			match++;
			//ps_process *ps = new ps_process(pid, find_name);
			std::unique_ptr<ps_process> ps = std::make_unique<ps_process>(pid, find_name);
			//match_list.push_back(ps);
			ps->dump();
			match_list.push_back(std::move(ps));
			//ps->dump();
		}
	}
  }

    if(-1 == closedir(dp)) {
        cm_log::error( cm_util::format("/proc closedir failed: %s", strerror(errno)));
    }

  return match;
}

