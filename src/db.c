#include "todoctl/db.h"
#include "todoctl/errors.h"

static int __write_db_header(int fd) {
  if (fd < 0) {
    fprintf(stderr,
            "TodoCtl db file does not exist! Please initialize first.\n");
    return STATUS_ERROR;
  }

  db_header_t *header = (db_header_t *)calloc(1, sizeof(db_header_t));
  if (header == NULL) {
    fprintf(stderr, "Failed to alloc db_header");
    return STATUS_ERROR;
  }

  header->magic = htonl(DB_MAGIC);
  header->version = htonl(1);
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

  if (read(fd, header, sizeof(db_header_t)) != sizeof(db_header_t)) {
    perror("read()");
    free(header);
    return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->magic != DB_MAGIC) {
    fprintf(stderr, "Invalid magic for db header\n");
    free(header);
    return TODOCTL_ERR_INVALID_HEADER_MAGIC;
  }

  if (header->version != 1) {
    fprintf(stderr, "Invalid magic for db header\n");
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
  int fd = *_fd;
  if (_fd == NULL) {
    fd = open(DEFAULT_DB_PATH, O_RDWR);
    if (fd < 0) {
      fprintf(stderr,
              "TodoCtl db file does not exist! Please initialize first.\n");
      return TODOCTL_ERR_DB_DOES_NOT_EXIST;
    }
  }

  /* if the file exists validate if the headers are valid */
  if (__validate_db_header(fd) < 0) {
    fprintf(stderr,
            "Failed to validate DB header possibly corrupted or invalid.\n");
    if (_fd == NULL) {
      close(fd);
    }
    return STATUS_ERROR;
  }

  if (_fd == NULL) {
    close(fd);
  }
  return 0;
}

int create_new_todo_db(void) {
  wordexp_t exp_res;
  wordexp(DEFAULT_DB_PATH, &exp_res, 0);

  /* create a file O_EXCL makes sure if it already exists we won't overwrite it */
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
