/*
 * debug.h -- Debug utils
 *
 * Author: frostzt
 * Date: 2026-01-25
 */

#ifndef TODOCTL_DEBUG_H
#define TODOCTL_DEBUG_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_ERROR = 3,
  LOG_LEVEL_NONE = 4,
} log_level_t;

void debug_set_log_level(log_level_t level);

log_level_t debug_get_log_level(void);

#ifdef DEBUG
#define DEBUG_LOG(fmt, ...)                                                                        \
  debug_print(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...)                                                                       \
  debug_print(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define DEBUG_WARN(fmt, ...)                                                                       \
  debug_print(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define DEBUG_ERROR(fmt, ...)                                                                      \
  debug_print(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(buf, len, fmt, ...)                                                          \
  debug_hexdump(LOG_LEVEL_DEBUG, buf, len, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(...)                                                                             \
  do {                                                                                             \
  } while (0)
#define DEBUG_INFO(...)                                                                            \
  do {                                                                                             \
  } while (0)
#define DEBUG_WARN(...)                                                                            \
  do {                                                                                             \
  } while (0)
#define DEBUG_ERROR(...)                                                                           \
  do {                                                                                             \
  } while (0)
#define DEBUG_HEXDUMP(...)                                                                         \
  do {                                                                                             \
  } while (0)
#endif

void debug_print(log_level_t level, const char *file, int line, const char *func, const char *fmt,
                 ...);

void debug_hexdump(log_level_t level, const void *buf, size_t len, const char *file, int line,
                   const char *fmt, ...);

#define COLOR_DEBUG "\x1b[32m" // Green
#define COLOR_INFO "\x1b[36m"  // Cyan
#define COLOR_WARN "\x1b[33m"  // Yellow
#define COLOR_ERROR "\x1b[31m" // Red
#define COLOR_RESET "\x1b[0m"  // Reset

#endif // TODOCTL_DEBUG_H
