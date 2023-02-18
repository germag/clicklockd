/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdio.h>
#include <stdarg.h>

#include "clicklockd.h"

static LOGGER_LOG_LEVEL logger_log_level = LOGGER_LOG_INFO;

void logger_set_level(LOGGER_LOG_LEVEL level) {
    logger_log_level = level;
}

void logger_log(LOGGER_LOG_LEVEL level, const char *fmt, ...) {

    if (level > logger_log_level) return;

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}