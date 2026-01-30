#include "todoctl/entry.h"
#include "todoctl/db.h"
#include "todoctl/debug.h"
#include "todoctl/errors.h"
#include "todoctl/util.h"

int build_entry(const char *task, todo_entry_t **out) {
  if (task == NULL) { return STATUS_ERROR; }

  todo_entry_t *entry = malloc(sizeof(todo_entry_t));
  if (entry == NULL) {
    perror("malloc()");
    return STATUS_ERROR;
  }

  size_t task_len = strlen(task);
  entry->entry_raw_data = malloc(task_len + 1);
  if (entry->entry_raw_data == NULL) {
    perror("malloc()");
    free(entry);
    return STATUS_ERROR;
  }

  /* get last entry id from the db header */
  uint64_t last_entry = 0;
  if (get_last_entry(&last_entry) < 0) {
    free(entry);
    return STATUS_ERROR;
  }

  entry->entry_id = last_entry + 1;
  entry->_created_at = get_time_in_millis();
  entry->_deleted_at = 0;
  memcpy(entry->entry_raw_data, task, task_len + 1);
  entry->entry_raw_data_len = task_len;

  *out = entry;
  return 0;
}

int encode_entry(const todo_entry_t *entry, char *out, size_t out_size, size_t *bytes_written) {
  if (entry == NULL) return STATUS_ERROR;
  if (out == NULL) return STATUS_ERROR;
  if (bytes_written == NULL) return STATUS_ERROR;
  if (out_size == 0) return STATUS_ERROR;

  *bytes_written = 0;

  /* calculate required data sizes */
  uint32_t raw_data_length = entry->entry_raw_data_len;
  if (raw_data_length != strlen(entry->entry_raw_data)) {
    DEBUG_ERROR("invalid data length expected %d got %d\n", raw_data_length,
                strlen(entry->entry_raw_data));
    return STATUS_ERROR;
  }

  size_t required_size = 4 + 8 + 8 + 8 + 4 + raw_data_length;
  if (out_size < required_size) {
    DEBUG_ERROR("Buffer too small: need %zu bytes, have %zu\n", required_size, out_size);
    return TODOCTL_ERR_BUFFER_TOO_SMALL;
  }

  if (raw_data_length > MAX_TODO_TEXT_LENGTH) {
    DEBUG_ERROR("Todo text too long: %u bytes (max: %d)\n", raw_data_length, MAX_TODO_TEXT_LENGTH);
    return TODOCTL_ERR_TODO_TOO_LONG;
  }

  size_t offset = 0;

  /* assign zero we'll calculate and set this later */
  uint32_t total_length = 0;
  uint32_t total_length_net = htonl(total_length);
  memcpy(out + offset, &total_length_net, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  /* write the id */
  uint64_t entry_id = htonll(entry->entry_id);
  memcpy(out + offset, &entry_id, sizeof(uint64_t));
  offset += sizeof(uint64_t);

  /* write the timestamps */
  uint64_t created_at = htonll(entry->_created_at);
  memcpy(out + offset, &created_at, sizeof(uint64_t));
  offset += sizeof(uint64_t);
  uint64_t deleted_at = htonll(entry->_deleted_at);
  memcpy(out + offset, &deleted_at, sizeof(uint64_t));
  offset += sizeof(uint64_t);

  /* write the task */
  uint32_t data_len = htonl(raw_data_length);
  memcpy(out + offset, &data_len, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  if (raw_data_length > 0) {
    memcpy(out + offset, entry->entry_raw_data, raw_data_length);
    offset += raw_data_length;
  }

  /* update the total length */
  total_length = (uint32_t)offset;
  total_length_net = htonl(total_length);
  memcpy(out, &total_length_net, sizeof(uint32_t));

  *bytes_written = offset;
  return 0;
}

int print_entry(const todo_entry_t *entry) {
  if (entry == NULL) return STATUS_ERROR;
  printf("%llu: %s\n", entry->entry_id, entry->entry_raw_data);
  return 0;
}

int print_entries(const todo_entry_t **entries, size_t n) {
  if (n < 0) return -1;
  if (n == 0) return 0;

  for (size_t i = 0; i < n; i++) {
    print_entry(entries[i]);
  }

  return 0;
}

// ❯ xxd ~/.todo.db
// 00000000: 0000 0000 004e 4e4e 0000 0001 0000 003e  .....NNN.......>
//                               |LENGTH | | ENTRY
// 00000010: 0000 0000 0000 0001 0000 0026 0000 0000  ...........&....
//               ID  | |    CREATED AT   | | DELETED
// 00000020: 0000 0001 0000 019c 0fba aa70 0000 0000  ...........p....
//            AT     | |DATALEN| |    DATA    |
// 00000030: 0000 0000 0000 0006 736f 7572 6176       ........sourav
//                                s o  u r  a v
int read_entries_from_db(int fd) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd provided");
    return STATUS_ERROR;
  }

  /* the db header is always 24 bytes which we will skip over */
  if (lseek(fd, 24, SEEK_SET) < 0) {
    DEBUG_ERROR("failed to offset the cursor ahead of the header");
    return STATUS_ERROR;
  }

  uint8_t length_buf[4];
  if (read(fd, &length_buf, 4) != 4) {
#ifdef DEBUG
    perror("read()");
#endif
    DEBUG_ERROR("failed to read length from buffer");
    return STATUS_ERROR;
  }

  /* get total length */
  uint32_t total_length;
  memcpy(&total_length, length_buf, 4);
  total_length = ntohl(total_length);

  return 0;
}
