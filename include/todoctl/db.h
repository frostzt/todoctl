/*
 * db.h -- TodoCtl DB
 *
 * Author: frostzt
 * Date: 2026-01-25
 */

#ifndef TODOCTL_DB_H
#define TODOCTL_DB_H

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wordexp.h>

#define DB_MAGIC 0x4e4e4e
#define DEFAULT_DB_PATH "~/.todo.db"
#define DB_HEADER_VERSION 1

#define UPDATE_NONE 0x00
#define UPDATE_FILESIZE (1 << 0)      /* sets the filesize to the new value */
#define UPDATE_LAST_ENTRY (1 << 2)    /* update last entry id */
#define UPDATE_ENTRIES_COUNT (1 << 3) /* sets the last entry count */
#define UPDATE_FILESIZE_ADD (1 << 4)  /* if set then filesize will be added to the current value */
#define UPDATE_ENTRIES_COUNT_ADD (1 << 5) /* if set adds the provided value to the count */
#define UPDATE_ENTRIES_COUNT_INCR (1 << 6) /* if set increments the entries count by 1 */
#define UPDATE_ALL 0xFF

typedef struct {
  uint64_t magic;
  uint32_t version;
  uint32_t filesize;

  uint64_t _last_entry_id;
  uint32_t _entries;
} db_header_t;

/* validates if the db file already exists */
int validate_db_exists(int *);

/* creates and initializes a new db file for todoctl */
int create_new_todo_db(void);

/* reads the header into the given struct */
int read_header(int, db_header_t *);

/*----------------------------------------------------------------
 * Utils
 *----------------------------------------------------------------*/

/* gets the last entry that was created from the header */
int get_last_entry(uint64_t *);

/* updates the header with the updated entry as last entry
 *
 * Observe the `db_header_t` struct above we have the following:
 *
 * |  MAGIC  |  VERSION  |  FILE_SIZE  | _LAST_ENTRY_ID  | _ENTRIES |
 *   8 bytes    4 bytes     4 bytes         8 bytes         4 bytes
 *
 * The file will have the first 28 bytes as header always! The only
 * problem we have is that we need to update the 4 bytes in the end
 * to contain the updated id. This function does exactly that.
 *
 * Honestly NOT sure this is the best approach since the best way
 * would possibly be to fetch the last record and check it?
 * This is only possible because of the 8 bytes constraint otherwise
 * we will have to rewrite the entire file.
 *
 * MARKING IT UNSAFE for now. This will be updated. Note I wrote it
 * because I wanted to try something like this! THIS CAN CORRUPT DATA
 */
int __UNSAFE__update_last_entry(const uint64_t);

/* same as above I am not sure how would this guy be managed to be honest */
int __UNSAFE__update_file_size(const uint32_t, const bool);

/* more of a function that does what the above does but updates everything */
int __UNSAFE__update_db_header(int, const db_header_t *, int);

/*----------------------------------------------------------------
 * DB OPS
 *----------------------------------------------------------------*/

/* writes a buffer to the disk */
int write_to_db(char *, size_t);

#endif // TODOCTL_DB_H
