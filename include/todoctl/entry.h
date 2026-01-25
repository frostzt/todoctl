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
  uint64_t entry_id;
  char *entry_raw_data;

  uint64_t _created_at;
  uint64_t _deleted_at; /* a background thread will clean this up */
} todo_entry_t;

void create_entry(void);

#endif // TODOCTL_ENTRY_H
