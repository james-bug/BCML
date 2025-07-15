#ifndef BCML_LOG_H
#define BCML_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN  = 1,
    LOG_LEVEL_INFO  = 2,
    LOG_LEVEL_DEBUG = 3
} log_level_t;

// Set global log level
void bcml_set_log_level(log_level_t level);
// Get global log level
log_level_t get_log_level(void);

// Main logging output function
void bcml_printf(log_level_t level, const char* fmt, ...);

// Macros for simplified log usage
#define BCML_LOG_ERROR(fmt, ...) bcml_printf(LOG_LEVEL_ERROR, "[ERROR] " fmt, ##__VA_ARGS__)
#define BCML_LOG_WARN(fmt, ...)  bcml_printf(LOG_LEVEL_WARN,  "[WARN ] " fmt, ##__VA_ARGS__)
#define BCML_LOG_INFO(fmt, ...)  bcml_printf(LOG_LEVEL_INFO,  "[INFO ] " fmt, ##__VA_ARGS__)
#define BCML_LOG_DEBUG(fmt, ...) bcml_printf(LOG_LEVEL_DEBUG, "[DEBUG] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif