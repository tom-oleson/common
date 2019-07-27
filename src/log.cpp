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

#include "log.h"
#include "timewatcher.h"

static const char *log_level[] = CM_LOG_LEVEL_NAMES;

//-------------------------------------------------------------------------
// formatter(s)
//-------------------------------------------------------------------------

std::string cm_log::format_log_timestamp(const std::string &fmt, time_t seconds, bool gmt ) {
        char buf[sizeof "01/01/1970 00:00:00.999 "] = { '\0' };

        struct tm my_tm;
        if(gmt) {
        	gmtime_r(&seconds, &my_tm); // break down as GMT/UTC time
        } else {
        	localtime_r(&seconds, &my_tm); // break down as local time
        }
        //strftime(buf, sizeof buf, "%m/%d/%Y %H:%M:%S ", &my_tm);
	strftime(buf, sizeof buf, fmt.c_str(), &my_tm);
        return std::string(buf);
}

std::string cm_log::format_millis(time_t millis) {
	char buf[sizeof ".999 "] = { '\0' };
	snprintf(buf, sizeof buf, ".%03ld", millis);
	return std::string(buf);
}

//-------------------------------------------------------------------------
// build up log message with timestamp, level and log message
//-------------------------------------------------------------------------

std::string cm_log::build_log_message(
	 const std::string &date_time_fmt,
	 const std::string &log_fmt,
	 cm_log::level::en lvl,
	 const std::string &msg,
	 bool gmt) {
        std::stringstream ss;

	time_t seconds, millis;
	seconds = getTime(&millis, NULL);

	ss << format_log_timestamp(date_time_fmt, seconds, gmt);
	ss << format_millis(millis) << " ";
	ss << ::log_level[lvl] << " ";
	ss << msg;
	return ss.str();
}

//-------------------------------------------------------------------------
// default logger (output to the default logger)
//-------------------------------------------------------------------------
static cm_log::console_logger default_logger;


void cm_log::log(cm_log::level::en lvl, const std::string &msg) {

	// call the configured logger
	default_logger.log(lvl, msg);
}

void cm_log::log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) {

	// call the configured logger
	default_logger.log(loc, lvl, msg);
}

//-------------------------------------------------------------------------
// cm_log error (output to stderr)
//-------------------------------------------------------------------------


// used to output a message to stderr when things go wrong in the logger itself
void cm_log::_log_error(cm_log::src_loc loc, const std::string &msg) {
	fprintf(stderr, "LOG ERROR: [%s:%d:%s]: %s", loc.file, loc.line, loc.func, msg.c_str());
}

//-------------------------------------------------------------------------
// console logger (output to the terminal)
//-------------------------------------------------------------------------

void cm_log::console_logger::log(cm_log::level::en lvl, const std::string &msg) {

	if(!ok_to_log(lvl)) return;

	lock();
	fprintf(stdout, "%s: %s", ::log_level[lvl], msg.c_str());
	unlock();
}

void cm_log::console_logger::log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) {

	if(!ok_to_log(lvl)) return;

	lock();
	fprintf(stdout, "%s [%s:%d:%s]: %s", ::log_level[lvl], loc.file, loc.line, loc.func, msg.c_str());
	unlock();
}


//-------------------------------------------------------------------------
// file logger (output to file)
//-------------------------------------------------------------------------

void cm_log::file_logger::log(cm_log::level::en lvl, const std::string &msg) {

        if(!ok_to_log(lvl)) return;

        lock();
	open_log();

	*this << cm_log::build_log_message(date_time_fmt, msg_fmt, lvl, msg, gmt) << "\n";
	flush();

        unlock();
}

void cm_log::file_logger::log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) {

        if(!ok_to_log(lvl)) return;

        lock();
	open_log();

	std::stringstream ss(msg);
	ss << "[" << loc.file << ":" << loc.line << ":" << loc.func << "]: " << msg;
	*this << cm_log::build_log_message(date_time_fmt, msg_fmt, lvl, ss.str(), gmt) << "\n";

        unlock();
}
