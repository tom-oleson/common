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
static const char *log_part[] = CM_LOG_PART_NAMES;

//-------------------------------------------------------------------------
// message formatter
//-------------------------------------------------------------------------

int cm_log::get_part_index(const std::string &str) {

	try {
	  for(int i=0; log_part[i] != NULL; i++) {
		if(std::string(log_part[i]) == str) {
			return( i );
		}
	  }
	} CM_LOG_CATCH();

	return -1;	// no match
}


std::string cm_log::format_log_message(cm_log::extra ext, const std::string &date_time_fmt, std::vector<std::string> &msg_fmt, cm_log::level::en lvl, const std::string &msg, bool gmt) {

    std::stringstream ss;

    time_t seconds, millis;
    seconds = cm_time::clock_seconds(&millis, NULL);

    std::vector<std::string>::iterator p = msg_fmt.begin(); 
    while(p != msg_fmt.end()) {
        std::string &part = *p;
        // if we have a $n specifier
        if(part.size() > 1 && part[0] == '$' && std::isdigit((unsigned char)part[1])) {
            const char *pch = part.c_str();
            int n = std::atoi(&pch[1]);

            switch(n) {
                
                case cm_log::part::date_time:
                ss << cm_log::format_log_timestamp(date_time_fmt, seconds, gmt);
                break;

                case cm_log::part::millis:
                ss << cm_log::format_millis(millis);
                break;

                case cm_log::part::lvl:
                ss << ::log_level[lvl];
                break;

                case cm_log::part::msg:
                ss << msg;
                break;

                case cm_log::part::tz:
                ss << (gmt ? "Z" : cm_util::get_timezone_offset(seconds));
                break;
                    
                case cm_log::part::file:
                if(!ext.ignore()) ss << ext.file; 
                break;

                case cm_log::part::line:
                if(!ext.ignore()) ss << ext.line;
                break;

                case cm_log::part::func:
                if(!ext.ignore()) ss << ext.func;
                break;

                case cm_log::part::thread:
                ss << cm_util::tid();
                break;

                case cm_log::part::host:
                ss << cm_util::get_hostname();
                break;

                default:
                ss << "?error?";
                break;
            }
        }
        else {
            ss << part;
        }
        
        p++;
    }

    return ss.str();
}

// parse fmt string and output a vector of parts to use for log message
// output.
// fmt is of the form:
// "${date_time}${millis}${tz} ${lvl} <${file}:${line}:${func}> [${thread}]: ${msg}"
// output vector will have:
// { "$0", "$1", "$2", " ", "$3", " <", "$4", ":", "$5", ":", "$6", "> [", "$7", "]: ", "$9"}

void cm_log::parse_message_format(const std::string fmt, std::vector<std::string> &out_fmt) {

	char ch;
	std::string value;
    size_t index = 0;
	const char *buffer = fmt.c_str();
	while((ch = buffer[index])) {

		switch(ch) {
		    case '$':   // identifier
                if(buffer[index+1] == '{') {
					index += 2;
					while((ch = buffer[index]) && ch != '}') {
						value.append(1,ch);
						index++;
                    }
                    if(ch == '}') index++;
                    char s_buf[30] = { '\0' };
                    int part_index = cm_log::get_part_index(value);
                    if(part_index >= 0) {
                        // put it out as index number (i.e., "$0"...)
                        snprintf(s_buf, sizeof s_buf, "$%d", part_index);
                    }
                    else {
                        // not our format, put it back...
                        snprintf(s_buf, sizeof s_buf, "${%s}", value.c_str());
                    }
                    out_fmt.push_back(std::string(s_buf));
                    value.clear();
                    break;                    	
                }
                // fall through

            default:    // character
                value.append(1,ch);
                index++;
                if(buffer[index] == '$' || buffer[index] == '\0') {
                    out_fmt.push_back(value);
                    value.clear();
                }
                break;
		}

	}
}

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
// default logger (output to the default logger)
//-------------------------------------------------------------------------
static cm_log::console_logger default_logger;


void cm_log::log(cm_log::level::en lvl, const std::string &msg) {

	// call the configured logger
	default_logger.log(lvl, msg);
}

void cm_log::log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg) {

	// call the configured logger
	default_logger.log(ext, lvl, msg);
}

//-------------------------------------------------------------------------
// cm_log error (output to stderr)
//-------------------------------------------------------------------------


// used to output a message to stderr when things go wrong in the logger itself
void cm_log::_log_error(cm_log::extra ext, const std::string &msg) {
	fprintf(stderr, "LOG ERROR: [%s:%d:%s]: %s", ext.file, ext.line, ext.func, msg.c_str());
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

void cm_log::console_logger::log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg) {

	if(!ok_to_log(lvl)) return;

	lock();
	fprintf(stdout, "%s [%s:%d:%s]: %s", ::log_level[lvl], ext.file, ext.line, ext.func, msg.c_str());
	unlock();
}


//-------------------------------------------------------------------------
// file logger (output to file)
//-------------------------------------------------------------------------

void cm_log::file_logger::log(cm_log::level::en lvl, const std::string &msg) {

    if(!ok_to_log(lvl)) return;

    lock();
    open_log();

	*this << cm_log::format_log_message(cm_log::extra(), date_time_fmt, parsed_msg_fmt, lvl, msg, gmt) << "\n";
    flush();

    unlock();
}

void cm_log::file_logger::log(cm_log::extra ext, cm_log::level::en lvl, const std::string &msg) {

    if(!ok_to_log(lvl)) return;

    lock();
    open_log();

	//std::stringstream ss(msg);
    *this << cm_log::format_log_message(ext, date_time_fmt, parsed_msg_fmt, lvl, msg, gmt) << "\n"; 

    unlock();
}
