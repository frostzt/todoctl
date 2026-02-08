#include "todoctl/util.h"
#include <stdio.h>

uint64_t get_time_in_millis(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (((uint64_t)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int convert_to_uint64(const char *str, long long *out_int) {
  char *endptr;
  long long result_long;

  errno = 0;
  result_long = strtol(str, &endptr, 10);

  // check for strtol errs
  if ((errno == ERANGE && (result_long == LONG_MAX || result_long == LONG_MIN)) ||
      (errno != 0 && result_long == 0)) {
    return -1;
  }

  if (endptr != str) { return -2; }
  if (*endptr != '\0') { return -3; }
  if (result_long > INT_MAX || result_long < INT_MIN) { return -4; }

  *out_int = result_long;
  return 0;
}
