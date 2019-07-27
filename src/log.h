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

#ifndef __LOG_H
#define __LOG_H

#include <string>
#include <fstream>
#include <sstream>

#include "util.h"
#include "mutex.h"

namespace cm_log {

#define CM_LOG_LEVEL_OFF 0
#define CM_LOG_LEVEL_ALWAYS 1
#define CM_LOG_LEVEL_FATAL 2
#define CM_LOG_LEVEL_CRITICAL 3
#define CM_LOG_LEVEL_ERROR 4
#define CM_LOG_LEVEL_WARN 5
#define CM_LOG_LEVEL_INFO 6
#define CM_LOG_LEVEL_DEBUG 7
#define CM_LOG_LEVEL_TRACE 8

namespace level {
enum en {
    off = CM_LOG_LEVEL_OFF,
    always = CM_LOG_LEVEL_ALWAYS,
    fatal = CM_LOG_LEVEL_FATAL,
    critical = CM_LOG_LEVEL_CRITICAL,
    error = CM_LOG_LEVEL_ERROR,
    warning = CM_LOG_LEVEL_WARN,
    info = CM_LOG_LEVEL_INFO,
    debug = CM_LOG_LEVEL_DEBUG,
    trace = CM_LOG_LEVEL_TRACE
};

}//namespace level

#define CM_LOG_LEVEL_NAMES \
    { "off", "always", "fatal", "critical", "error", "warning", "info", "debug", "trace" }


struct src_loc {
    src_loc(): file{__FILE__}, line{__LINE__}, func{__FUNCTION__} {}
    src_loc(const char *_file, int _line, const char *_func):
         file{_file}, line{_line}, func{_func} {}

    const char *file{nullptr};
    int line{0};
    const char *func{nullptr};
};


#define SRC_LOC cm_log::src_loc(__FILE__, __LINE__, __FUNCTION__)

void log(cm_log::level::en lvl, const std::string &msg);
void log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg);

#define __LOG_SOURCE_LOC__
#ifdef __LOG_SOURCE_LOC__

#define always(msg) log(SRC_LOC, cm_log::level::always, msg)
#define fatal(msg) log(SRC_LOC, cm_log::level::fatal, msg)
#define critical(msg) log(SRC_LOC, cm_log::level::critical, msg)
#define error(msg) log(SRC_LOC, cm_log::level::error, msg)
#define warning(msg) log(SRC_LOC, cm_log::level::warning, msg)
#define info(msg) log(SRC_LOC, cm_log::level::info, msg)
#define debug(msg) log(SRC_LOC, cm_log::level::debug, msg)
#define trace(msg) log(SRC_LOC, cm_log::level::trace, msg)

#else

#define always(msg) log(cm_log::level::always, msg)
#define fatal(msg) log(cm_log::level::fatal, msg)
#define critical(msg) log(cm_log::level::critical, msg)
#define error(msg) log(m_log::level::error, msg)
#define warning(msg) log(m_log::level::warning, msg)
#define info(msg) log(cm_log::level::info, msg)
#define debug(msg) log(cm_log::level::debug, msg)
#define trace(msg) log(cm_log::level::trace, msg)

#endif

#define __CM_LOG_CATCH()\
    catch (const std::exception &ex) {_log_error(src_loc(__FILE__, __LINE__, __FUNCTION__), ex.what()); }\
    catch (...) { _log_error(src_loc(__FILE__, __LINE__, __FUNCTION__), "Unknown exception"); }

#define CM_LOG_PARTS \
{ "{date_time}", "{millis}", "{lvl}", "{file}", "{line}", "{func}", "{thread}", "{host}", "{msg}" } 


void _log_error(src_loc loc, const std::string &msg);


class logger {

protected:
	cm_log::level::en log_level;
	bool gmt;
	std::string date_time_fmt;	// see strftime()
	std::string msg_fmt;

public:
	logger():
		log_level(cm_log::level::info),
		gmt(false),
		date_time_fmt("%m/%d/%Y %H:%M:%S"),
		msg_fmt("{date_time}.{millis} {lvl} <{file}:{line}:{func}> [{thread}]: {msg}") {
	}

	void set_log_level(cm_log::level::en lvl) { log_level = lvl; }
	cm_log::level::en get_log_level(void) { return log_level; }
	void set_gmt(bool b) { gmt = b; }
	bool get_fmt(void) { return gmt; }

	bool ok_to_log(cm_log::level::en lvl) {
		return lvl != cm_log::level::off && lvl <= log_level;
	}

	
	virtual void log(cm_log::level::en lvl, const std::string &msg) = 0;
	virtual void log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) = 0;
};


class console_logger : public logger, private cm::mutex {
	
public:
	console_logger(): logger() { }
	~console_logger() { }

	void log(cm_log::level::en lvl, const std::string &msg);
	void log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg);		
};


class file_logger : public logger, private cm::mutex, private std::ofstream {

protected:
	std::string log_path;

	void open_log() {
		if(!is_open()) {
			open(log_path, ios_base::out | ios_base::app);
		}
	}

	void close_log() {
		if(is_open()) {
			close();
		}
	}

public:
	file_logger(const std::string path): logger(), log_path(path) { open_log(); }
	~file_logger() { lock(); close_log(); unlock(); }

	void log(cm_log::level::en lvl, const std::string &msg);
        void log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg); 	
};


std::string format_log_timestamp(const std::string &fmt, time_t seconds, /*time_t millis,*/ bool gmt);
std::string format_millis(time_t millis);
std::string build_log_message( const std::string &date_time_fmt, const std::string &log_fmt,
         cm_log::level::en lvl, const std::string &msg, bool gmt);

} // namespace cm_log



#endif	// __LOG_H

