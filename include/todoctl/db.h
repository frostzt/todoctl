/*
 * db.h -- TodoCtl DB
 *
 * Author: frostzt
 * Date: 2026-01-25
 */

#ifndef TODOCTL_DB_H
#define TODOCTL_DB_H

#include <fcntl.h>
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

/* gets the last entry that was created from the header */
uint64_t get_last_entry(void);

/*----------------------------------------------------------------
 * DB OPS
 *----------------------------------------------------------------*/

/* writes a buffer to the disk */
int write_to_db(char *, size_t);

#endif // TODOCTL_DB_H
