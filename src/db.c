#include "todoctl/db.h"
#include "todoctl/errors.h"

static int __write_db_header(int fd) {
  if (fd < 0) {
    fprintf(stderr, "TodoCtl db file does not exist! Please initialize first.\n");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    fprintf(stderr, "Failed to alloc db_header");
    return STATUS_ERROR;
  }

  /* this method will be called only when creating the file
   * for the frist time so creating the last entry as 0 should
   * be fine to do */
  header->_last_entry_id = htonl(0);
  header->magic = htonl(DB_MAGIC);
  header->version = htons(DB_HEADER_VERSION);

  // TODO: Need to add Entry size here
  header->filesize = htons(sizeof(db_header_t));

  if (write(fd, header, sizeof(db_header_t)) < 0) {
    perror("write()");
    free(header);
    return STATUS_ERROR;
  }

  return 0;
}

static int __validate_db_header(int fd) {
  if (fd < 0) {
    fprintf(stderr, "Bad fd for the db file.\n");
    return STATUS_ERROR;
  }
  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    fprintf(stderr, "Failed to alloc db_header\n");
    return STATUS_ERROR;
  }

  read(fd, header, sizeof(db_header_t));
  header->_last_entry_id = ntohl(header->_last_entry_id);
  header->version = ntohs(header->version);
  header->magic = ntohl(header->magic);
  header->filesize = ntohs(header->filesize);

  if (header->magic != DB_MAGIC) {
    fprintf(stderr, "Invalid magic for db header\n");
    free(header);
    return TODOCTL_ERR_INVALID_HEADER_MAGIC;
  }

  if (header->version != DB_HEADER_VERSION) {
    fprintf(stderr, "Invalid db version\n");
    free(header);
    return TODOCTL_ERR_INVALID_VERSION;
  }

  struct stat buffer = {0};
  fstat(fd, &buffer);
  if (header->filesize != buffer.st_size) {
    fprintf(stderr, "Corrupted db file.\n");
    free(header);
    return TODOCTL_ERR_CORRUPTED_DB;
  }

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
    fprintf(stderr, "Failed to validate DB header possibly corrupted or invalid.\n");
    if (_fd == NULL) { close(fd); }
    return STATUS_ERROR;
  }

  if (_fd == NULL) { close(fd); }
  return 0;
}

int create_new_todo_db(void) {
  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);

  /* create a file O_EXCL makes sure if it already exists we won't overwrite it
   */
  int fd = open(exp_res.we_wordv[0], O_RDWR | O_CREAT | O_EXCL, 0644);
  wordfree(&exp_res);
  if (fd < 0) {
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
  header->_last_entry_id = ntohl(header->_last_entry_id);
  header->version = ntohl(header->version);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  uint64_t last_entry_id = header->_last_entry_id;
  free(header);
  close(fd);
  *value = last_entry_id;
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
