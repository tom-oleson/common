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

#ifndef __COLOR_H
#define __COLOR_H

#pragma once

//ANSI color escape sequences for interactive terminal output

namespace cm_color {

inline void put(const char *s) { printf("%s", s);}

#define CM_COLOR_RESET      "\x1B[0m"
#define CM_COLOR_BOLD       "\x1B[1m"
#define CM_COLOR_UNDERSCORE "\x1B[4m"   // monochrome only 
#define CM_COLOR_BLINK      "\x1B[5m"
#define CM_COLOR_REVERSE    "\x1B[7m"
#define CM_COLOR_CONCEAL    "\x1B[8m"   

#define CM_FG_BLACK   "\x1B[30m"
#define CM_FG_RED     "\x1B[31m"
#define CM_FG_GREEN   "\x1B[32m"
#define CM_FG_YELLOW  "\x1B[33m"
#define CM_FG_BLUE    "\x1B[34m"
#define CM_FG_MAGENTA "\x1B[35m"
#define CM_FG_CYAN    "\x1B[36m"
#define CM_FG_WHITE   "\x1B[37m"

#define CM_BG_BLACK   "\x1B[40m"
#define CM_BG_RED     "\x1B[41m"
#define CM_BG_GREEN   "\x1B[42m"
#define CM_BG_YELLOW  "\x1B[43m"
#define CM_BG_BLUE    "\x1B[44m"
#define CM_BG_MAGENTA "\x1B[45m"
#define CM_BG_CYAN    "\x1B[46m"
#define CM_BG_WHITE   "\x1B[47m"

} // namespace cm_sio

#endif
