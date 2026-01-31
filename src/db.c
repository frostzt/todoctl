#include "todoctl/db.h"
#include "todoctl/debug.h"
#include "todoctl/errors.h"

static int __write_db_header(int fd) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd received");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    DEBUG_ERROR("failed to alloc db_header");
    return STATUS_ERROR;
  }

  /* this method will be called only when creating the file
   * for the frist time so creating the last entry as 0 should
   * be fine to do */
  header->_last_entry_id = htonll(0);
  header->magic = htonll(DB_MAGIC);
  header->version = htonl(DB_HEADER_VERSION);
  header->filesize = htonl(sizeof(db_header_t));
  header->_entries = htonl(0);

  if (write(fd, header, sizeof(db_header_t)) < 0) {
    DEBUG_ERROR("write(): failed to alloc db_header");
    perror("write()");
    free(header);
    return STATUS_ERROR;
  }

  return 0;
}

int read_header(int fd, db_header_t *out_header) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd provided\n");
    return STATUS_ERROR;
  }

  if (read(fd, out_header, sizeof(db_header_t)) < 0) {
    DEBUG_ERROR("failed to read db header\n");
    return STATUS_ERROR;
  }

  out_header->_last_entry_id = ntohll(out_header->_last_entry_id);
  out_header->magic = ntohll(out_header->magic);
  out_header->version = ntohl(out_header->version);
  out_header->filesize = ntohl(out_header->filesize);
  out_header->_entries = ntohl(out_header->_entries);
  return 0;
}

static int __validate_db_header(int fd) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd provided\n");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    DEBUG_ERROR("failed to alloc db header\n");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(db_header_t)) < 0) {
    DEBUG_ERROR("failed to read db header\n");
    return STATUS_ERROR;
  }

  header->magic = ntohll(header->magic);
  header->version = ntohl(header->version);
  header->filesize = ntohl(header->filesize);

  if (header->magic != DB_MAGIC) {
    DEBUG_ERROR("invalid magic db header\n");
    free(header);
    return TODOCTL_ERR_INVALID_HEADER_MAGIC;
  }

  if (header->version != DB_HEADER_VERSION) {
    DEBUG_ERROR("invalid db version\n");
    free(header);
    return TODOCTL_ERR_INVALID_VERSION;
  }

  /* TODO: Now filesize is complicated as we have stuff written */
  // struct stat buffer = {0};
  // fstat(fd, &buffer);
  // if (header->filesize != buffer.st_size) {
  //   DEBUG_ERROR("corrupted db file\n");
  //   free(header);
  //   return TODOCTL_ERR_CORRUPTED_DB;
  // }

  free(header);
  return 0;
}

int validate_db_exists(int *_fd) {
  int fd;
  if (_fd == NULL) {
    wordexp_t exp_res;
    wordexp(DEFAULT_DB_PATH, &exp_res, 0);
    fd = open(exp_res.we_wordv[0], O_RDWR);
    wordfree(&exp_res);
    if (fd < 0) {
      fprintf(stderr, "TodoCtl db file does not exist! Please initialize first.\n");
      return TODOCTL_ERR_DB_DOES_NOT_EXIST;
    }
  } else {
    fd = *_fd;
  }

  /* if the file exists validate if the headers are valid */
  if (__validate_db_header(fd) < 0) {
    // fprintf(stderr, "Failed to validate DB header possibly corrupted or invalid.\n");
    if (_fd == NULL) { close(fd); }
    return STATUS_ERROR;
  }

  if (_fd == NULL) { close(fd); }
  return 0;
}

int create_new_todo_db(void) {
  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);

  /* create a file O_EXCL makes sure if it already exists we won't overwrite it */
  int fd = open(exp_res.we_wordv[0], O_RDWR | O_CREAT | O_EXCL, 0644);
  wordfree(&exp_res);
  if (fd < 0) {
    if (errno == EEXIST) {
      fprintf(stderr, "TODO DB already exists.\n");
      return TODOCTL_ERR_FAILED_DB_CREATE;
    }

    perror("open()");
    return TODOCTL_ERR_FAILED_DB_CREATE;
  }

  if (__write_db_header(fd) < 0) {
    fprintf(stderr, "Failed to write db headers.\n");
    return STATUS_ERROR;
  }

  close(fd);
  return 0;
}

int get_last_entry(uint64_t *value) {
  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);

  /* we'll assume that the file exists */
  int fd = open(exp_res.we_wordv[0], O_RDONLY);
  wordfree(&exp_res);
  if (fd < 0) {
    perror("open()");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    fprintf(stderr, "Failed to alloc db_header\n");
    close(fd);
    return STATUS_ERROR;
  }

  read(fd, header, sizeof(db_header_t));
  header->_last_entry_id = ntohll(header->_last_entry_id);
  *value = header->_last_entry_id;

  free(header);
  close(fd);
  return 0;
}

int __UNSAFE__update_file_size(const uint32_t filesize, const bool add) {
  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);

  /* we'll assume that the file exists */
  int fd = open(exp_res.we_wordv[0], O_RDWR);
  wordfree(&exp_res);
  if (fd < 0) {
    perror("open()");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    DEBUG_ERROR("failed to alloc db_header\n");
    close(fd);
    return STATUS_ERROR;
  }

  read(fd, header, sizeof(db_header_t));
  header->filesize = ntohl(header->filesize);

  if (lseek(fd, 12, SEEK_SET) < 0) {
    DEBUG_ERROR("failed to lseek\n");
    free(header);
    close(fd);
    return STATUS_ERROR;
  }

  uint32_t filesize_endian = 0;
  if (add) {
    filesize_endian = htonl(header->filesize + filesize);
  } else {
    filesize_endian = htonl(filesize);
  }

  if (write(fd, &filesize_endian, 4) != 4) {
    DEBUG_ERROR("failed to write into filesize\n");
    free(header);
    close(fd);
    return STATUS_ERROR;
  }

  return 0;
}

int __UNSAFE__update_db_header(int fd, const db_header_t *update, int flags) {
  if (fd < 0) {
    DEBUG_ERROR("invalid fd provided\n");
    return STATUS_ERROR;
  }

  const size_t SKIP_FOR_FILE_SIZE = 12;
  const size_t SKIP_FOR_LAST_ENTRY = 16;
  const size_t SKIP_FOR_TOTAL_ENTRIES = 24;

  if (flags == UPDATE_NONE) { return 0; }
  db_header_t *header = (db_header_t *)malloc(sizeof(db_header_t));
  if (header == NULL) {
    DEBUG_ERROR("failed to alloc db_header_t\n");
#ifdef DEBUG
    perror("malloc()");
#endif
    return STATUS_ERROR;
  }

  /* read into header */
  if (read(fd, header, 28) != 28) {
    DEBUG_ERROR("failed to read into header\n");
#ifdef DEBUG
    perror("read()");
#endif
    free(header);
    return STATUS_ERROR;
  }

  header->magic = ntohll(header->magic);
  header->version = ntohl(header->version);
  header->filesize = ntohl(header->filesize);
  header->_last_entry_id = ntohll(header->_last_entry_id);
  header->_entries = ntohl(header->_entries);

  /* update file size */
  if (flags & UPDATE_FILESIZE) {
    uint32_t new_file_size = update->filesize;
    if (flags & UPDATE_FILESIZE_ADD) { new_file_size = header->filesize + new_file_size; }
    if (lseek(fd, SKIP_FOR_FILE_SIZE, SEEK_SET) != SKIP_FOR_FILE_SIZE) {
      DEBUG_ERROR("failed to seek for updating filesize\n");
#ifdef DEBUG
      perror("lseek()");
#endif
      free(header);
      return STATUS_ERROR;
    }

    new_file_size = htonl(new_file_size);
    if (write(fd, &new_file_size, 4) != 4) {
      DEBUG_ERROR("failed to write update for filesize\n");
#ifdef DEBUG
      perror("lseek()");
#endif
      free(header);
      return STATUS_ERROR;
    }
  }

  /* update last entry */
  if (flags & UPDATE_LAST_ENTRY) {
    uint64_t new_last_entry = update->_last_entry_id;
    if (lseek(fd, SKIP_FOR_LAST_ENTRY, SEEK_SET) != SKIP_FOR_LAST_ENTRY) {
      DEBUG_ERROR("failed to seek for updating last entry\n");
#ifdef DEBUG
      perror("lseek()");
#endif
      free(header);
      return STATUS_ERROR;
    }

    new_last_entry = htonll(new_last_entry);
    if (write(fd, &new_last_entry, 8) != 8) {
      DEBUG_ERROR("failed to write update for last entry\n");
#ifdef DEBUG
      perror("lseek()");
#endif
      free(header);
      return STATUS_ERROR;
    }
  }

  /* update total entries */
  if (flags & UPDATE_ENTRIES_COUNT) {
    uint32_t new_total_entries = update->_entries;
    if (lseek(fd, SKIP_FOR_TOTAL_ENTRIES, SEEK_SET) != SKIP_FOR_TOTAL_ENTRIES) {
      DEBUG_ERROR("failed to seek for updating total entries\n");
#ifdef DEBUG
      perror("lseek()");
#endif
      free(header);
      return STATUS_ERROR;
    }

    if (flags & UPDATE_ENTRIES_COUNT_ADD) {
      new_total_entries = header->_entries + update->_entries;
    }

    if (flags & UPDATE_ENTRIES_COUNT_INCR) { new_total_entries = header->_entries + 1; }

    new_total_entries = htonl(new_total_entries);
    if (write(fd, &new_total_entries, 4) != 4) {
      DEBUG_ERROR("failed to write update for last entry\n");
#ifdef DEBUG
      perror("lseek()");
#endif
      free(header);
      return STATUS_ERROR;
    }
  }

  return 0;
}

int __UNSAFE__update_last_entry(const uint64_t last_record_id) {
  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);

  /* we'll assume that the file exists */
  int fd = open(exp_res.we_wordv[0], O_RDWR);
  wordfree(&exp_res);
  if (fd < 0) {
    perror("open()");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)malloc(sizeof(db_header_t));
  if (header == NULL) {
    fprintf(stderr, "Failed to alloc db_header\n");
    close(fd);
    return STATUS_ERROR;
  }

  if (lseek(fd, 16, SEEK_SET) < 0) {
    free(header);
    close(fd);
    return STATUS_ERROR;
  }

  const uint64_t last_record_id_endian = htonll(last_record_id);
  if (write(fd, &last_record_id_endian, 8) != 8) {
    free(header);
    close(fd);
    return STATUS_ERROR;
  }

  return 0;
}

int write_to_db(char *buf, size_t n) {
  if (buf == NULL) {
    fprintf(stderr, "Empty buffer provided.");
    return STATUS_ERROR;
  }

  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);
  /* open in append mode */
  FILE *fp = fopen(exp_res.we_wordv[0], "a");
  wordfree(&exp_res);
  if (fp == NULL) {
    perror("fopen()");
    return STATUS_ERROR;
  }

  /* write to the stream, append into the file */
  if (fwrite(buf, 1, n, fp) < n) {
    perror("fwrite()");
    fclose(fp);
    return STATUS_ERROR;
  }

  if (fclose(fp) != 0) {
    perror("fclose()");
    return STATUS_ERROR;
  }

  return 0;
}
