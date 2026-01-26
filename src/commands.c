#include "todoctl/commands.h"
#include "todoctl/db.h"
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
  /* update the header with the latest byte */
  if (__UNSAFE__update_last_entry(entry->entry_id) < 0) { return STATUS_ERROR; }

  return 0;
}

int list_tasks_command(void) {
  if (validate_db_exists(NULL) < 0) { return STATUS_ERROR; }

  return 0;
}
