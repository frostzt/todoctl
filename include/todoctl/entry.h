/*
 * entry.h -- TodoCtl entry
 *
 * Author: frostzt
 * Date: 2026-01-25
 */

#ifndef TODOCTL_ENTRY_H
#define TODOCTL_ENTRY_H

#include <stdint.h>
#include <time.h>

typedef struct {
  int64_t entry_id;
  char *entry_raw_data;
} todo_entry_t;

#endif // TODOCTL_ENTRY_H
