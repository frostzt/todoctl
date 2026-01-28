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

typedef struct {
  uint64_t magic;
  uint32_t version;
  uint32_t filesize;

  uint64_t _last_entry_id;
} db_header_t;

/* validates if the db file already exists */
int validate_db_exists(int *_fd);

/* creates and initializes a new db file for todoctl */
int create_new_todo_db(void);

/*----------------------------------------------------------------
 * Utils
 *----------------------------------------------------------------*/

/* gets the last entry that was created from the header */
int get_last_entry(uint64_t *);

/* updates the header with the updated entry as last entry
 *
 * Observe the `db_header_t` struct above we have the following:
 *
 * |  MAGIC  |  VERSION  |  FILE_SIZE  | _LAST_ENTRY_ID  |
 *   8 bytes    4 bytes     4 bytes         8 bytes
 *
 * The file will have the first 24 bytes as header always! The only
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

/*----------------------------------------------------------------
 * DB OPS
 *----------------------------------------------------------------*/

/* writes a buffer to the disk */
int write_to_db(char *, size_t);

#endif // TODOCTL_DB_H
