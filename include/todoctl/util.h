/*
 * util.h -- Utilities
 *
 * Author: frostzt
 * Date: 2026-01-25
 */

#ifndef TODOCTL_UTIL_H
#define TODOCTL_UTIL_H

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

/*
 * Gets current time in millis
 * @see https://stackoverflow.com/a/44896326/12034976
 */
uint64_t get_time_in_millis(void);

/* converts a string safely to a long long */
int convert_to_uint64(const char *, long long *);

#endif // TODOCTL_UTIL_H
