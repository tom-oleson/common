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

#ifndef __UTIL_H
#define __UTIL_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <regex>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>      /* for gettid() */
#include <sys/stat.h>		/* for stat() */
#include <dirent.h>         /* for readdir() */
#include <stdarg.h>
#include <sys/time.h>
#include <float.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>


namespace cm_util {

std::string format(const char *fmt, ...);
std::string &format_string(std::string& s, const char *fmt, ...);
size_t bin2hex(const unsigned char *bin, size_t bin_len, char *hex, size_t hex_len, bool lowercase = false);
std::string format_local_timestamp(time_t seconds, time_t millis, std::string &tz);
std::string format_utc_timestamp(time_t seconds, time_t millis);
std::string format_filename_timestamp(time_t seconds, bool gmt);
std::string get_timezone_offset(time_t seconds);
std::string get_hostname();
pid_t tid();

int file_stat(const std::string &path, size_t *size, time_t *mod_time);
int rename(const std::string &old_name, const std::string &new_name);
int remove(const std::string &path);
int dir_scan(const std::string &dir_name, const std::string &pattern, std::vector<std::string> &matches);


bool append_to_file(const std::string &path, const std::string &str);

time_t calendar_time(time_t seconds, struct tm &local_tm);
time_t next_midnight(time_t seconds);
time_t prev_midnight(time_t seconds);
time_t next_hour(time_t seconds, int n_hour);
time_t next_interval(time_t seconds, time_t interval);
time_t next_calendar_time(time_t seconds, int hour, int min, int sec);

size_t strlcpy(char *dst, const char *src, size_t max);

static const char *hex_upper = "0123456789ABCDEF";
static const char *hex_lower = "0123456789abcdef";


inline void byte2hex(const unsigned char byte, char hex[], const char *digits) {
	hex[0]  = digits[byte >> 4];
    hex[1] = digits[byte & 0x0F];
}

void bin2hex_line(char *out_buf, int out_len, const void *src_addr, const int src_len, const int width, const char *digits);

std::vector<std::string> split (const std::string &s, char delim);
 
} // namespace cm_util


#endif
