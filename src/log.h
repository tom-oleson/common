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
#include "util.h"

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

#define off(msg) log(SRC_LOC, cm_log::level::off, msg)
#define always(msg) log(SRC_LOC, cm_log::level::always, msg)
#define fatal(msg) log(SRC_LOC, cm_log::level::fatal, msg)
#define critical(msg) log(SRC_LOC, cm_log::level::critical, msg)
#define error(msg) log(SRC_LOC, cm_log::level::error, msg)
#define warn(msg) log(SRC_LOC, cm_log::level::warn, msg)
#define info(msg) log(SRC_LOC, cm_log::level::info, msg)
#define debug(msg) log(SRC_LOC, cm_log::level::debug, msg)
#define trace(msg) log(SRC_LOC, cm_log::level::trace, msg)

#else

#define off(msg) log(cm_log::level::off, msg)
#define always(msg) log(cm_log::level::always, msg)
#define fatal(msg) log(cm_log::level::fatal, msg)
#define critical(msg) log(cm_log::level::critical, msg)
#define error(msg) log(m_log::level::error, msg)
#define warn(msg) log(m_log::level::warn, msg)
#define info(msg) log(cm_log::level::info, msg)
#define debug(msg) log(cm_log::level::debug, msg)
#define trace(msg) log(cm_log::level::trace, msg)

#endif

#define __CM_LOG_CATCH()\
    catch (const std::exception &ex) {_log_error(src_loc(__FILE__, __LINE__, __FUNCTION__), ex.what()); }\
    catch (...) { _log_error(src_loc(__FILE__, __LINE__, __FUNCTION__), "Unknown exception"); }

void _log_error(src_loc loc, const std::string &msg);

#endif

class logger {

	cm_log::level::en log_level;

public:
	logger(): log_level(cm_log::level::info) {}

	void set_log_level(cm_log::level::en lvl) { log_level = lvl; }
	cm_log::level::en get_log_level(void) { return log_level; }

	
	virtual void log(cm_log::level::en lvl, const std::string &msg) = 0;
	virtual void log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg) = 0;
};


class console_logger : public logger {
	
public:
	console_logger(): logger() { }
	~console_logger() { }

	void log(cm_log::level::en lvl, const std::string &msg);
	void log(cm_log::src_loc loc, cm_log::level::en lvl, const std::string &msg);		
};


} // namespace cm_log
