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
  entry->_done_at = 0;
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

  size_t required_size = /* Length */ 4 + /* entry id */ 8 + /* created at */ 8 +
                         /* deleted at */ 8 + /* done at */ 8 + /* data length */ 4 +
                         raw_data_length;
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
  uint64_t done_at = htonll(entry->_done_at);
  memcpy(out + offset, &done_at, sizeof(uint64_t));
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
  if (n == 0) return 0;
  if (entries == NULL) return STATUS_ERROR;

  for (size_t i = 0; i < n; i++) {
    print_entry(entries[i]);
  }
  return 0;
}

int update_entry_done(int fd, const db_header_t *header, const uint64_t entry_id) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd provided\n");
    return STATUS_ERROR;
  }

  todo_entry_t **entries = malloc(sizeof(todo_entry_t *) * header->_entries);
  if (entries == NULL) {
    DEBUG_ERROR("failed to allocate entries\n");
#ifdef DEBUG
    perror("malloc()");
#endif
    return STATUS_ERROR;
  }

  /* read entries from the db */
  uint64_t stop_at = entry_id;
  size_t bytes_read = 0;
  /* this will be the last entry read and is the one we need to mark */
  int entries_read = read_entries_from_db(fd, header, entries, &bytes_read, &stop_at);
  if (entries_read <= 0) {
    DEBUG_ERROR("failed to read entries from db\n");
    /* free all the entries */
    for (size_t i = 0; i < header->_entries; i++) {
      free(entries[i]->entry_raw_data);
      free(entries[i]);
    }
    return STATUS_ERROR;
  }

  size_t last_entry_data_len = entries[entries_read - 1]->entry_raw_data_len;
  size_t total_seek = (sizeof(db_header_t) + bytes_read) - (last_entry_data_len + 4 + 8);

  if (lseek(fd, total_seek, SEEK_SET) < 0) {
    DEBUG_ERROR("failed to seek for setting the entry done\n");
#ifdef DEBUG
    perror("lseek()");
#endif

    /* free all the entries */
    for (size_t i = 0; i < header->_entries; i++) {
      free(entries[i]->entry_raw_data);
      free(entries[i]);
    }
    return STATUS_ERROR;
  }

  uint64_t done_at = htonll(get_time_in_millis());
  if (write(fd, &done_at, 8) != 8) {
    DEBUG_ERROR("failed to write update for last entry\n");
#ifdef DEBUG
    perror("write()");
#endif

    /* free all the entries */
    for (size_t i = 0; i < header->_entries; i++) {
      free(entries[i]->entry_raw_data);
      free(entries[i]);
    }
    return STATUS_ERROR;
  }

  return 0;
}

// ✦ ❯ xxd ~/.todo.db
//           | MAGIC           | |VERSION| |FILESZ |
// 00000000: 0000 0000 004e 4e4e 0000 0001 0000 0020  .....NNN.......
//           | LAST ENTRY ID   | |ENTRIES| PADDING
// 00000010: 0000 0000 0000 0002 0000 0002 0000 0000  ................
//           | LENGTH| | ENTRY ID        | | CREATED
// 00000020: 0000 0026 0000 0000 0000 0001 0000 019c  ...&............
//           AT      | |    DELETED AT   | |DATALEN|
// 00000030: 13f9 780e 0000 0000 0000 0000 0000 0006  ..x.............
//           |  SOURAV    | | repeats
// 00000040: 736f 7572 6176 0000 0026 0000 0000 0000  sourav...&......
// 00000050: 0002 0000 019c 13fa 1537 0000 0000 0000  .........7......
// 00000060: 0000 0000 0006 736f 7572 6176            ......sourav
int read_entries_from_db(int fd, const db_header_t *header, todo_entry_t **entries,
                         size_t *bytes_read, uint64_t *stopat) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd provided\n");
    return STATUS_ERROR;
  }
  if (header == NULL) {
    DEBUG_ERROR("header is NULL\n");
    return STATUS_ERROR;
  }
  if (entries == NULL) {
    DEBUG_ERROR("entries array is NULL\n");
    return STATUS_ERROR;
  }

  if (lseek(fd, sizeof(db_header_t), SEEK_SET) < 0) {
    DEBUG_ERROR("failed to offset the cursor ahead of the header\n");
    return STATUS_ERROR;
  }

  /* track the amount of bytes we're reading */
  if (bytes_read != NULL) { *bytes_read = 0; }

  /* loop over and get all the entries */
  size_t i = 0;
  for (; i < header->_entries; i++) {
    uint8_t length_buf[4];
    if (read(fd, &length_buf, 4) != 4) {
#ifdef DEBUG
      perror("read()");
#endif
      DEBUG_ERROR("failed to read length from buffer\n");
      return STATUS_ERROR;
    }
    if (bytes_read != NULL) { *bytes_read += 4; }

    /* get total length */
    uint32_t total_length;
    memcpy(&total_length, length_buf, 4);
    total_length = ntohl(total_length);

    todo_entry_t *entry = (todo_entry_t *)malloc(sizeof(todo_entry_t));
    if (entry == NULL) {
#ifdef DEBUG
      perror("malloc()");
#endif
      DEBUG_ERROR("failed to alloc entry\n");
      return STATUS_ERROR;
    }

    uint8_t buffer[36];
    /* read from entry_id to data len all into the buffer */
    if (read(fd, &buffer, 36) != 36) {
#ifdef DEBUG
      perror("read()");
#endif
      DEBUG_ERROR("failed to read entry id from buffer\n");
      return STATUS_ERROR;
    }
    if (bytes_read != NULL) { *bytes_read += 36; }

    /* parse entry id from the buffer */
    uint64_t entry_id;
    memcpy(&entry_id, buffer, 8);
    entry->entry_id = ntohll(entry_id);

    /* parse created_at from the buffer */
    uint64_t created_at;
    memcpy(&created_at, buffer + 8, 8);
    entry->_created_at = ntohll(created_at);

    /* parse deleted_at from the buffer */
    uint64_t deleted_at;
    memcpy(&deleted_at, buffer + 16, 8);
    entry->_deleted_at = ntohll(deleted_at);

    /* parse done_at from the buffer */
    uint64_t done_at;
    memcpy(&done_at, buffer + 24, 8);
    entry->_done_at = ntohll(done_at);

    /* parse data_len from the buffer */
    uint32_t data_len;
    memcpy(&data_len, buffer + 32, 4);
    data_len = ntohl(data_len);

    /* match if the total length matches the actual bytes */
    size_t expected = 4 + 8 + 8 + 8 + 8 + 4 + data_len;
    if (total_length != expected) {
      DEBUG_ERROR("Corrupted entry: length mismatch\n");
      return STATUS_ERROR;
    }

    /* allocate space for the string */
    entry->entry_raw_data = malloc(data_len + 1);
    if (entry->entry_raw_data == NULL) {
#ifdef DEBUG
      perror("malloc()");
#endif
      free(entry);
      DEBUG_ERROR("failed alloc raw data bytes\n");
      return STATUS_ERROR;
    }

    if (read(fd, entry->entry_raw_data, data_len) != data_len) {
#ifdef DEBUG
      perror("read()");
#endif
      free(entry->entry_raw_data);
      free(entry);
      DEBUG_ERROR("failed to read raw string into buffer\n");
      return STATUS_ERROR;
    }
    entry->entry_raw_data[data_len] = '\0';
    entry->entry_raw_data_len = (size_t)data_len;
    if (bytes_read) { *bytes_read += data_len; }

    entries[i] = entry;

    /* check if we wanna stop here */
    if (stopat != NULL && *stopat == entry->entry_id) { break; }
  }

  if (stopat != NULL) { return (int)i; }

  return 0;
}
