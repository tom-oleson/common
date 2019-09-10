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

#include <exception>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "util.h"
#include "mutex.h"
#include "timewatcher.h"


extern void *default_logger;

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
    { "off", "always", "fatal", "critical", "error", "warning", "info", "debug", "trace", NULL }


struct extra {
    extra() {}
    extra(const char *_file, int _line, const char *_func):
         file{_file}, line{_line}, func{_func} {}

    const char *file{nullptr};
    int line{0};
    const char *func{nullptr};

    bool ignore() { return line == 0; }
};

#define CM_LOG_EXTRA cm_log::extra(__FILE__, __LINE__, __FUNCTION__)

void log(cm_log::level::en lvl, const std::string &msg);
void log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg);

void _hex_dump(cm_log::level::en lvl, const void *buf, int sz);
void _hex_dump(cm_log::extra ext, cm_log::level::en lvl, const void *buf, int sz);


#define log_always(logger) if(logger.ok_to_log(cm_log::level::always))
#define log_fatal(logger) if(logger.ok_to_log(cm_log::level::fatal))
#define log_critical(logger) if(logger.ok_to_log(cm_log::level::critical))
#define log_error(logger) if(logger.ok_to_log(cm_log::level::error))
#define log_warning(logger) if(logger.ok_to_log(cm_log::level::warning))
#define log_info(logger) if(logger.ok_to_log(cm_log::level::info))
#define log_debug(logger) if(logger.ok_to_log(cm_log::level::debug))
#define log_trace(logger) if(logger.ok_to_log(cm_log::level::trace))

#define CM_LOG_ALWAYS log_always(get_default_logger())
#define CM_LOG_FATAL log_fatal(get_default_logger())
#define CM_LOG_CRITICAL log_critical(get_default_logger())
#define CM_LOG_ERROR log_error(get_default_logger())
#define CM_LOG_WARNING log_warning(get_default_logger())
#define CM_LOG_INFO log_info(get_default_logger())
#define CM_LOG_DEBUG log_debug(get_default_logger())
#define CM_LOG_TRACE log_trace(get_default_logger())


#define __LOG_EXTRA__
#ifdef __LOG_EXTRA__

#define always(msg) log(CM_LOG_EXTRA, cm_log::level::always, msg)
#define fatal(msg) log(CM_LOG_EXTRA, cm_log::level::fatal, msg)
#define critical(msg) log(CM_LOG_EXTRA, cm_log::level::critical, msg)
#define error(msg) log(CM_LOG_EXTRA, cm_log::level::error, msg)
#define warning(msg) log(CM_LOG_EXTRA, cm_log::level::warning, msg)
#define info(msg) log(CM_LOG_EXTRA, cm_log::level::info, msg)
#define debug(msg) log(CM_LOG_EXTRA, cm_log::level::debug, msg)
#define trace(msg) log(CM_LOG_EXTRA, cm_log::level::trace, msg)

#define hex_dump(lvl, buf, buf_sz) _hex_dump(CM_LOG_EXTRA, lvl, buf, buf_sz)


#else

#define always(msg) log(cm_log::level::always, msg)
#define fatal(msg) log(cm_log::level::fatal, msg)
#define critical(msg) log(cm_log::level::critical, msg)
#define error(msg) log(m_log::level::error, msg)
#define warning(msg) log(m_log::level::warning, msg)
#define info(msg) log(cm_log::level::info, msg)
#define debug(msg) log(cm_log::level::debug, msg)
#define trace(msg) log(cm_log::level::trace, msg)

#define hex_dump(lvl, buf, buf_sz) _hex_dump(lvl, buf, buf_sz)

#endif

#define CM_LOG_CATCH()\
    catch (const std::exception &ex)\
	{ cm_log::_log_error(CM_LOG_EXTRA, ex.what()); }\
    catch (...)\
	{ cm_log::_log_error(CM_LOG_EXTRA, "Unknown exception"); }

#define CM_LOG_DATE_TIME 0
#define CM_LOG_MILLIS 1
#define CM_LOG_TZ 2
#define CM_LOG_LVL 3
#define CM_LOG_FILE 4
#define CM_LOG_LINE 5
#define CM_LOG_FUNC 6
#define CM_LOG_THREAD 7
#define CM_LOG_HOST 8
#define CM_LOG_MSG 9

enum part {
    date_time = CM_LOG_DATE_TIME,
    millis = CM_LOG_MILLIS,
    tz = CM_LOG_TZ,
    lvl = CM_LOG_LVL,
    file = CM_LOG_FILE,
    line = CM_LOG_LINE,
    func = CM_LOG_FUNC,
    thread = CM_LOG_THREAD,
    host = CM_LOG_HOST,
    msg = CM_LOG_MSG
};

#define CM_LOG_PART_NAMES \
{ "date_time", "millis", "tz", "lvl", "file", "line", "func", "thread", "host", "msg", NULL } 


void _log_error(extra ext, const std::string &msg);
std::string format_log_message(cm_log::extra, const std::string &date_time_fmt, std::vector<std::string>& fmt, cm_log::level::en, const std::string&, bool gmt);
int get_part_index(const std::string &str);
void parse_message_format(const std::string fmt, std::vector<std::string> &out_fmt);

std::string format_log_timestamp(const std::string &fmt, time_t seconds, /*time_t millis,*/ bool gmt);
std::string format_millis(time_t millis);
std::string format_log_message( const std::string &date_time_fmt, const std::string &log_fmt,
         cm_log::level::en lvl, const std::string &msg, bool gmt);


class logger {

protected:
    std::string name;
	cm_log::level::en log_level;
	bool gmt;
	std::string date_time_fmt;	// see strftime()
	std::string msg_fmt;
    std::vector<std::string> parsed_msg_fmt;

    void *save_default_logger;

public:
	logger():
		log_level(cm_log::level::info), gmt(false), date_time_fmt("%m/%d/%Y %H:%M:%S"),
		msg_fmt("${date_time} [${lvl}]: ${msg}") {
        cm_log::parse_message_format(msg_fmt, parsed_msg_fmt);

        // save pointer to default logger to be restored
        save_default_logger = default_logger;
	}

    ~logger() {
        // restore default logger if we became "the one"
        if(default_logger == this) {
            default_logger = save_default_logger;
        }
    }

	void set_log_level(cm_log::level::en lvl) { log_level = lvl; }
	cm_log::level::en get_log_level(void) { return log_level; }
    void set_date_time_format(std::string fmt) { date_time_fmt = fmt; }
    void set_message_format(std::string fmt) {
        msg_fmt = fmt;
        parsed_msg_fmt.clear();
        cm_log::parse_message_format(msg_fmt, parsed_msg_fmt);
    }
	void set_gmt(bool b) { gmt = b; }
	bool get_gmt(void) { return gmt; }

	bool ok_to_log(cm_log::level::en lvl) {
		return (lvl != cm_log::level::off && lvl <= log_level) || lvl == cm_log::level::always;
	}
	
	virtual void log(cm_log::level::en lvl, const std::string &msg) = 0;
	virtual void log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg) = 0;
};


class console_logger : public logger, protected cm::mutex {
	
public:
	console_logger(): logger() { name = "console-logger"; }
	~console_logger() { }

	void log(cm_log::level::en lvl, const std::string &msg);
	void log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg);		
};


class file_logger : public logger, protected cm::mutex, protected std::ofstream {

protected:
	std::string log_path;

	void open_log() {
		if(!is_open()) {
			open(log_path, ios_base::out | ios_base::app);
		}
	}

	void close_log() {
		if(is_open()) {
            flush();
			close();
		}
	}

public:
    file_logger() { name = "file-logger"; }
	file_logger(const std::string path): logger(), log_path(path) {
        lock(); open_log(); unlock();
    }
	~file_logger() { lock(); close_log(); unlock(); }

	void log(cm_log::level::en lvl, const std::string &msg);
    void log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg); 	
};


class roller {

protected:
    // rotate every interval seconds from midnight
    // note: interval in seconds must be an even multiple of 24 hours (24*60*60)
    time_t interval = 0;  /* rotate every interval seconds from midnight */
    time_t next_rotate_time = 0;
    time_t this_rotate_time = 0;

public:
    roller() { set_interval(24 * 60 * 60); }
    roller(time_t _interval) { set_interval( _interval); } 
    ~roller() {}        

    void set_interval(time_t _interval) { 
        interval = _interval;
        if( !((24 * 60 * 60)  % interval == 0) )
            interval = 24 * 60 * 60;
        
        time_t now = cm_time::clock_seconds();
        time_t diff = now - cm_util::prev_midnight(now);

        int intervals = diff / interval + ( diff % interval != 0 ? 1 : 0);
        next_rotate_time = now + ((interval * intervals) - diff);
    }

    bool check_to_rotate() {
        time_t seconds = cm_time::clock_seconds();
        if(seconds >= next_rotate_time) {
            this_rotate_time = next_rotate_time;
            next_rotate_time = this_rotate_time + interval;
            rotate();
            return true;
        }
        return false;
    }    

    virtual void rotate() = 0;
};


class rolling_file_logger : public file_logger, public roller  {

protected:
    std::string dir;
    std::string base_name;
    std::string ext;

    int keep = 0;       // # of logs to keep in rotation (0 = remove none)

    std::vector<std::string> rotation_list;


    // build path (e.g., dir="./some_path/", base_name="app", ext=".log" becomes
    // "./some_path/app.log")
    std::string build_path(const std::string _dir, const std::string _base_name,
         const std::string _ext) {

        dir = _dir;
        base_name = _base_name;
        ext = _ext;

        return (dir +  base_name + ext);
    }

    // build target path (e.g. "./some_path/20190806_134518_app.log")
    std::string build_rotate_path(const std::string timestamp) {
        return (dir + timestamp + (gmt ? "Z" : "") + "_" + base_name + ext);
    }

public:
    rolling_file_logger(): roller() { name = "rolling-file-logger"; }
    rolling_file_logger(const std::string _dir, const std::string _base_name,
             const std::string _ext, time_t _interval, int _keep = 0): file_logger(), roller(_interval), keep(_keep) {
        log_path = build_path(_dir, _base_name, _ext);
        lock(); open_log(); unlock();
    }

    ~rolling_file_logger() { }

    void rotate();

    void log(cm_log::level::en lvl, const std::string &msg);
    void log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg);
};


class multiplex_logger : public logger {

protected:
    std::vector<logger *> loggers;

public:
    multiplex_logger(): logger() { name = "multiplex-logger"; }
    ~multiplex_logger() { loggers.clear(); }

    // apply date_time format for all member loggers
    void set_date_time_format(std::string fmt);

    // apply message format for all member loggers
    void set_message_format(std::string fmt); 

    // add a member logger to multiplex_logger
    void add(cm_log::logger &_logger) { loggers.push_back(&_logger); }

    // log message to all member loggers
    void log(cm_log::level::en lvl, const std::string &msg);
    void log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg);
};


extern console_logger console;


} // namespace cm_log

inline cm_log::logger &get_default_logger() { return *(cm_log::logger*)default_logger; }
inline void set_default_logger(cm_log::logger *_logger) {
    default_logger = _logger;
}

#endif	// __LOG_H

