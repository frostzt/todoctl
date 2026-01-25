#include "todoctl/util.h"
#include <sys/time.h>

uint64_t time_in_millis(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (((uint64_t)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}
