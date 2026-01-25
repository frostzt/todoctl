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
#include <sys/stat.h>
#include <unistd.h>
#include <wordexp.h>

#define DB_MAGIC 0x4e4e4e
#define DEFAULT_DB_PATH "~/.todo.db"

typedef struct {
  uint64_t magic;
  uint32_t version;
  uint32_t filesize;
} db_header_t;

/* validates if the db file already exists */
int validate_db_exists(int *_fd);

/* creates and initializes a new db file for todoctl */
int create_new_todo_db(void);

#endif // TODOCTL_DB_H
