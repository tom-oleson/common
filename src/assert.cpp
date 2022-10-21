/*
 * Copyright (c) 2022, Tom Oleson <tom dot oleson at gmail dot com>
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

#include <cstdio>
#include <stdlib.h>
#include "assert.h"

namespace cm_assert {

void assert(const char *exp, const char *file_name, unsigned line) {
    cm_log::error(cm_util::format("ASSERT: %s: Assertion failed: %s, line %u", exp, file_name, line));
}

void assert_abort(const char *exp, const char *file_name, unsigned line) {
    cm_log::error(cm_util::format("ASSERT_ABORT: %s: Assertion failed: %s, line %u", exp, file_name, line));
	abort();
}

bool unit_test(bool result, const char *exp, const char *file_name, unsigned line) {
	if(result) {
        cm_log::info(cm_util::format("UNIT_TEST: %s: %s, line %u: PASS", exp, file_name, line));
    }
    else {
        cm_log::error(cm_util::format("UNIT_TEST: %s: %s, line %u: FAIL", exp, file_name, line));

    }
    return result;
}


}
