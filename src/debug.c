#include "todoctl/debug.h"

static log_level_t current_log_level = LOG_LEVEL_DEBUG;

void debug_print(log_level_t level, const char *file, int line, const char *func, const char *fmt,
                 ...) {
  if (current_log_level > level) { return; }

  /* get timestamp */
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  /* get file name */
  const char *basename = strrchr(file, '/');
  if (basename) {
    basename++;
  } else {
    basename = file;
  }

  /* set color */
  const char *color = "";
  const char *level_str = "";
  FILE *output = stdout;

  int use_color = isatty(fileno(stdout)) || isatty(fileno(stderr));

  /* update config based on log level */
  switch (level) {
  case LOG_LEVEL_DEBUG:
    level_str = "DEBUG";
    color = use_color ? COLOR_DEBUG : "";
    output = stdout;
    break;
  case LOG_LEVEL_INFO:
    level_str = "INFO";
    color = use_color ? COLOR_INFO : "";
    output = stdout;
    break;
  case LOG_LEVEL_WARN:
    level_str = "DEBUG";
    color = use_color ? COLOR_WARN : "";
    output = stderr;
    break;
  case LOG_LEVEL_ERROR:
    level_str = "ERROR";
    color = use_color ? COLOR_ERROR : "";
    output = stderr;
    break;
  default:
    return;
  }

  /* construct and print header */
  fprintf(output, "%s[%s] [%s] [%s:%d] [%s] ", color, timestamp, level_str, basename, line, func);

  /* messages */
  va_list args;
  va_start(args, fmt);
  vfprintf(output, fmt, args);
  va_end(args);

  fprintf(output, "%s\n", use_color ? COLOR_RESET : "");
  fflush(output);
}

void debug_hexdump(log_level_t level, const void *buf, size_t len, const char *file, int line,
                   const char *fmt, ...) {
  if (current_log_level > level) { return; }

  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  const char *basename = strrchr(file, '/');
  basename = basename ? basename + 1 : file;

  const char *color = "";
  FILE *output = stdout;
  int use_color = isatty(fileno(stdout));

  if (use_color) { color = COLOR_DEBUG; }

  fprintf(output, "%s[%s] [DEBUG] [%s:%d] ", color, timestamp, basename, line);

  va_list args;
  va_start(args, fmt);
  vfprintf(output, fmt, args);
  va_end(args);

  fprintf(output, " (%zu bytes):%s\n", len, use_color ? COLOR_RESET : "");

  const uint8_t *data = (const uint8_t *)buf;
  for (size_t i = 0; i < len; i += 16) {
    fprintf(output, "%04zx:  ", i);

    for (size_t j = 0; j < 16; j++) {
      if (i + j < len) {
        fprintf(output, "%02x ", data[i + j]);
      } else {
        fprintf(output, "   ");
      }

      if (j == 7) { fprintf(output, " "); }
    }

    fprintf(output, " ");
    for (size_t j = 0; j < 16 && i + j < len; j++) {
      uint8_t c = data[i + j];
      fprintf(output, "%c", (c >= 32 && c <= 126) ? c : '.');
    }

    fprintf(output, "\n");
  }

  fflush(output);
}

void debug_set_log_level(log_level_t level) {
  current_log_level = level;
}

log_level_t debug_get_log_level(void) {
  return current_log_level;
}
