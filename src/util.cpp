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

std::string cm_util::format_local_timestamp(time_t seconds, time_t millis, std::string &tz) {
	char buf[sizeof "1970-01-01T00:00:00.999+00:00"] = { '\0' };
	struct tm local_tm;
	localtime_r(&seconds, &local_tm); // break down as local time
	strftime(buf, sizeof buf, "%FT%T", &local_tm);
	snprintf(buf + 19, sizeof buf - 19, ".%03ld%s", millis, tz.c_str());
	return std::string(buf);	
}

std::string cm_util::format_utc_timestamp(time_t seconds, time_t millis) {
	char buf[sizeof "1970-01-01T00:00:00.999Z"] = { '\0' };
	struct tm utc_tm;
	gmtime_r(&seconds, &utc_tm);	// break down as UTC time
	strftime(buf, sizeof buf, "%FT%T", &utc_tm);
	snprintf(buf + 19, sizeof buf - 19, ".%03ldZ", millis);
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

pid_t cm_util::pid() {
#define __LINUX_GETTID__
#ifdef __LINUX_GETTID__
    pid_t pid = syscall(SYS_gettid); // kernel id (linux)
#else
    pid_t pid = syscall(SYS_lwp_self);  // non-linux
#endif
}

