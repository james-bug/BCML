#include "bcml_log.h"
#include <stdio.h>
#include <stdarg.h>

static log_level_t g_log_level = LOG_LEVEL_DEBUG;

// Set the global log level
void bcml_set_log_level(log_level_t level) {
    g_log_level = level;
}

// Get the current global log level
log_level_t get_log_level(void) {
    return g_log_level;
}

// Logging output function with log level filtering
void bcml_printf(log_level_t level, const char* fmt, ...) {
    if (level > g_log_level)
        return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}