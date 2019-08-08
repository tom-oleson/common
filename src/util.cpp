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

#include "util.h"

#define _SPRINTF_BUF_SZ 4096

//-------------------------------------------------------------------------
// printf style string formatter 
//-------------------------------------------------------------------------
std::string &cm_util::format(std::string& s, const char *fmt, ...) {
    char c_format_buf[_SPRINTF_BUF_SZ];
    memset(c_format_buf, 0, sizeof(c_format_buf));

    va_list args;
    va_start(args, fmt);
    int sz = vsnprintf(c_format_buf,sizeof(c_format_buf),fmt,args);
    va_end(args);

    return s.assign(c_format_buf,sz);
}

//-------------------------------------------------------------------------
// Convert binary bytes to hex-ASCII.
//-------------------------------------------------------------------------

size_t cm_util::bin2hex(const unsigned char *bin, size_t bin_len, char *hex, size_t hex_len) {
 
	size_t out_size = bin_len * 2 + 1;

        if (bin == NULL || bin_len == 0 || hex == NULL ||
                hex_len == 0 || (hex_len < out_size)) {
                return 0;
        }

        memset(hex, 0, hex_len);

        size_t j = 0;
        const char *digits = "0123456789ABCDEF";
        for (size_t i = 0; i < bin_len; i++) {
                j = i * 2;
                hex[j]   = digits[bin[i] >> 4];
                hex[j+1] = digits[bin[i] & 0x0F];
        }
        hex[out_size-1] = '\0';

        return out_size;
}

time_t cm_util::next_midnight(time_t seconds) {

    struct tm local_tm;
    local_tm.tm_isdst = -1;

    time_t tomorrow = seconds + (24 * 60 * 60);  // 24 hours from now
    localtime_r(&tomorrow, &local_tm);   // break down as local time

    // back next day up to its midnight
    local_tm.tm_sec = 0;
    local_tm.tm_min = 0;
    local_tm.tm_hour = 0;

    // compute and return the epoch time this will happen
    return mktime(&local_tm);
}

time_t cm_util::prev_midnight(time_t seconds) {

    struct tm local_tm;
    local_tm.tm_isdst = -1;

    localtime_r(&seconds, &local_tm);   // break down as local time

    // move it to midnight
    local_tm.tm_sec = 0;
    local_tm.tm_min = 0;
    local_tm.tm_hour = 0;

    // compute and return the epoch time this will happened
    return mktime(&local_tm);
}

time_t cm_util::next_interval(time_t seconds, time_t interval) {

    struct tm local_tm;
    local_tm.tm_isdst = -1;

    time_t future = seconds + interval;  // interval seconds from now
    localtime_r(&future, &local_tm);   // break down as local time

    // compute and return the epoch time this will happen
    return mktime(&local_tm);
}

time_t cm_util::next_hour(time_t seconds, int n_hour) {

    struct tm local_tm;
    local_tm.tm_isdst = -1;

    time_t future = seconds + (n_hour * (60 * 60));  // n_hour(s) from now
    localtime_r(&future, &local_tm);   // break down as local time

    // adjust to top of the hour
    local_tm.tm_sec = 0;
    local_tm.tm_min = 0;

    // compute and return the epoch time this will happen
    return mktime(&local_tm);
}

time_t cm_util::next_calendar_time(time_t seconds, int hour, int min, int sec) {

    struct tm local_tm;
    local_tm.tm_isdst = -1;

    time_t today = seconds;         // time now
    localtime_r(&today, &local_tm); // break down as local time
    local_tm.tm_sec = sec;
    local_tm.tm_min = min;
    local_tm.tm_hour = hour;
    today = mktime(&local_tm);      // time's epoch for today

    if(today > seconds) return today;   // next time point is today

    time_t tomorrow =  seconds + (24 * 60 * 60);  // 24 hours from now
    localtime_r(&tomorrow, &local_tm); // break down as local time
    local_tm.tm_sec = sec;
    local_tm.tm_min = min;
    local_tm.tm_hour = hour;
    tomorrow =  mktime(&local_tm);  // time's epoch for tomorrow

    return tomorrow;    // next time point is tomorrow
}

time_t cm_util::calendar_time(time_t seconds, struct tm &local_tm) {

    local_tm.tm_isdst = -1;
    localtime_r(&seconds, &local_tm); // break down as local time
    return mktime(&local_tm);
}

// use cm_util::get_timezone_offset() for tz value (we avoid it inside to reduce calls and correct tz offset format)
std::string cm_util::format_local_timestamp(time_t seconds, time_t millis, std::string &tz) {
	char buf[sizeof "1970-01-01T00:00:00.999+00:00"] = { '\0' };
	struct tm local_tm;
	localtime_r(&seconds, &local_tm); // break down as local time
	strftime(buf, sizeof buf, "%FT%T", &local_tm);
	snprintf(buf + 19, sizeof buf - 19, ".%03ld%s", millis, tz.c_str());
	return std::string(buf);	
}

std::string cm_util::format_utc_timestamp(time_t seconds, time_t millis) {
	char buf[sizeof "1970-01-01T00:00:00.999Z "] = { '\0' };
	struct tm utc_tm;
	gmtime_r(&seconds, &utc_tm);	// break down as UTC time
	strftime(buf, sizeof buf, "%FT%T", &utc_tm);
	snprintf(buf + 19, sizeof buf - 19, ".%03ldZ", millis);
	return std::string(buf);
}

// used to create timestamp part of a file name (e.g., 20190804_225819)
// useful for building output file names that need to be uniqued
std::string cm_util::format_filename_timestamp(time_t seconds, bool gmt) {
    char buf[sizeof "19700101_000000_000 "] = { '\0' };
    struct tm _tm;
    if(gmt) gmtime_r(&seconds, &_tm);    // break down as UTC time
    else localtime_r(&seconds, &_tm);   // break down as local time
    strftime(buf, sizeof buf, "%Y%m%d_%H%M%S", &_tm);
    return std::string(buf);
}

// returns the timezone offset formatted as: +HH:MM (see RFC 3339 and ISO 8601)
// or empty string if the TZ environment variable is missing (or not correctly configured)
std::string cm_util::get_timezone_offset(time_t seconds) {

	struct tm local_tm;
	localtime_r(&seconds, &local_tm); // break down as local time
	
    char buf[sizeof "+hhmm"] = {'\0'};	// strftime %z does not output ":"
    char tzoffset[sizeof "+hh:mm"] = {'\0'};	// RFC 3339/ISO 8601 format

	tzset();	// make sure TZ has been processed

    strftime(buf, sizeof buf, "%z", &local_tm);
	if(strlen(buf) == sizeof buf -1) {
		tzoffset[0] = buf[0];
		tzoffset[1] = buf[1];
		tzoffset[2] = buf[2];
		tzoffset[3] = ':';
		tzoffset[4] = buf[3];
		tzoffset[5] = buf[4];
		tzoffset[6] = '\0';
	}

	return std::string(tzoffset);
}

std::string cm_util::get_hostname() {
    char buf[80] = { '\0' };
    memset(buf, 0, sizeof buf);
    gethostname(buf, sizeof buf);
    return std::string(buf);
}

pid_t cm_util::tid() {
#define __LINUX_GETTID__
#ifdef __LINUX_GETTID__
    pid_t pid = syscall(SYS_gettid); // linux threads
#else
    pid_t pid = syscall(SYS_lwp_self);  // non-linux threads (solaris)
#endif
}

int cm_util::file_stat(const std::string &path, size_t *size, time_t *mod_time) {
	int ret = -1;
	struct stat info;
	
	if((ret = stat(path.c_str(), &info)) == 0) {
		if(S_ISREG(info.st_mode)) {     // if regular file
			if(NULL != size) {
				*size = info.st_size;
			}
			if(NULL != mod_time) {
				*mod_time = info.st_mtim.tv_sec;
			}
		}
	}
	
	return ret;
}

int cm_util::rename(const std::string &old_name, const std::string &new_name) {
    return std::rename(old_name.c_str(), new_name.c_str()); 
}

int cm_util::remove(const std::string &path) {
    return std::remove(path.c_str());
}

// append a string to a text file
// intended as a helper for unit tests only
bool cm_util::append_to_file(const std::string &path, const std::string &str) {

    std::ofstream fs;
    fs.open(path, std::ios_base::out | std::ios_base::app);
    if(!fs.is_open()) {
        return false;
    }
    fs << str.c_str() << "\n";

    fs.flush();
    fs.close();
    return true;
}

int cm_util::dir_scan(const std::string &dir_name, const std::string &pattern,
            std::vector<std::string> &matches) {
    int retcode = 0;
    DIR *dp;
    struct dirent *d;

    if (NULL == (dp = opendir(dir_name.c_str()))) {
        return -3;   // call strerror(errno) for error
    }

    try {
        const std::regex reg(pattern.c_str());
        while (NULL != (d = readdir(dp))) {
            const std::string name(d->d_name);
            if(std::regex_match(name, reg)) {
                matches.push_back(name);
            }
        }
    } catch(...) { retcode = -2; }

    if(-1 == closedir(dp)) {
        return -1;  // call strerror(errno) for error
    }

    return retcode;
}

