#include "todoctl/commands.h"
#include "todoctl/db.h"
#include "todoctl/debug.h"
#include "todoctl/entry.h"
#include "todoctl/errors.h"

int add_task_command(const char *task) {
  if (validate_db_exists(NULL) < 0) { return STATUS_ERROR; }

  /* build this entry from the user's task */
  todo_entry_t *entry = NULL;
  if (build_entry(task, &entry) < 0) { return STATUS_ERROR; }

  /* encode and flush into the database */
  char encoded_buffer[MAX_TODO_TEXT_LENGTH];
  size_t bytes_written = 0;
  if (encode_entry(entry, encoded_buffer, MAX_TODO_TEXT_LENGTH, &bytes_written)) {
    return STATUS_ERROR;
  }
  if (write_to_db(encoded_buffer, bytes_written) < 0) { return STATUS_ERROR; }

  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);
  int fd = open(exp_res.we_wordv[0], O_RDWR);
  wordfree(&exp_res);
  if (fd < 0) {
    DEBUG_ERROR("failed to open db file\n");
#ifdef DEBUG
    perror("open()");
#endif
    return STATUS_ERROR;
  }

  /* update the header with the details */
  db_header_t header;
  header._entries = 1;
  header._last_entry_id = entry->entry_id;
  header.filesize = (uint32_t)bytes_written;
  __UNSAFE__update_db_header(fd, &header,
                             UPDATE_LAST_ENTRY | UPDATE_FILESIZE_ADD | UPDATE_ENTRIES_COUNT_INCR);

  close(fd);
  return 0;
}

int list_tasks_command(const size_t limit) {
  if (validate_db_exists(NULL) < 0) { return STATUS_ERROR; }
  printf("%zu", limit);

  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);
  int fd = open(exp_res.we_wordv[0], O_RDONLY);
  wordfree(&exp_res);
  if (fd < 0) {
    DEBUG_ERROR("failed to open db file\n");
    return STATUS_ERROR;
  }

  /* read into header */
  db_header_t *header = (db_header_t *)malloc(sizeof(db_header_t));
  if (read_header(fd, header) < 0) { return STATUS_ERROR; }
  read_entries_from_db(fd);

  close(fd);
  return 0;
}
