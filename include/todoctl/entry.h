/*
 * entry.h -- TodoCtl entry
 *
 * Author: frostzt
 * Date: 2026-01-25
 */

#ifndef TODOCTL_ENTRY_H
#define TODOCTL_ENTRY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef htonll
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32)
#define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32)
#else
#define htonll(x) (x)
#define ntohll(x) (x)
#endif
#endif

#define ENTRY_FIXED_SIZE (sizeof(uint64_t) * 3) // id + created + deleted = 24 bytes

#define MAX_TODO_TEXT_LENGTH 4096
#define TEXT_LENGTH_PREFIX sizeof(uint32_t) // 4 bytes

/* total = 24 + 4 + 4096 = 4124 bytes */
#define ENCODED_ENTRY_MAX_SIZE (ENTRY_FIXED_SIZE + TEXT_LENGTH_PREFIX + MAX_TODO_TEXT_LENGTH)

typedef struct {
  uint64_t entry_id;
  char *entry_raw_data;

  uint64_t _created_at;
  uint64_t _deleted_at; /* a background thread will clean this up, 0 means not
                           deleted */
} todo_entry_t;

/* builds a new todo entry */
int build_entry(const char *, todo_entry_t **);

/* converts an entry into its binary encoded form
 * encodes this entry to be written directly into the file
 * on a disk this is how an entry looks like
 *
 * |LENGTH |ENTRY_ID|CREATED_AT|DELETED_AT|DATA_LEN|RAW_DATA|
 * |4 bytes|8 bytes | 8 bytes  | 8 bytes  |4 bytes | N bytes|
 *         ^                              ^
 *         |       ENTRY FIXED SIZE       |
 * ^       ^
 * |  TLP  |                              ^                 ^
 *                                        |  MAX TTLength   |
 */
int encode_entry(const todo_entry_t *, char *, size_t, size_t *);

#endif // TODOCTL_ENTRY_H
