/*
 * Copyright (c) 2019, Tom Oleson <tom dot oleson at gmail dot com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <float.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>

#include "log.h"

//-------------------------------------------------------------------------

static const char *log_level[] = CM_LOG_LEVEL_NAMES;
static cm_log::console_logger the_logger;


void cm_log::log(cm_log::level::en lvl, const std::string &msg) {

	// call the configured logger
	the_logger.log(lvl, msg);
}

void cm_log::log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) {

	// call the configured logger
	the_logger.log(loc, lvl, msg);
}


// used to output a message to stderr when things go wrong in the logger itself
void cm_log::_log_error(cm_log::src_loc loc, const std::string &msg) {
	fprintf(stderr, "LOG ERROR: [%s:%d:%s]: %s", loc.file, loc.line, loc.func, msg.c_str());
}

// console logger

void cm_log::console_logger::log(cm_log::level::en lvl, const std::string &msg) {
	fprintf(stdout, "%s: %s", ::log_level[lvl], msg.c_str());
}

void cm_log::console_logger::log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) {
	fprintf(stdout, "%s [%s:%d:%s]: %s", ::log_level[lvl], loc.file, loc.line, loc.func, msg.c_str());
}



