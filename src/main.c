#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "todoctl/db.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-a <task>] [-i]\n", argv[0]);
  printf("\t -i initialize todoctl\n");
  printf("\t -a adds a new task\n");
}

int main(int argc, char *argv[]) {
  int opt;

  /* parse flags right now `init` is a flag and does not take
   * an argument will have to think on how to approach this */
  while ((opt = getopt(argc, argv, "ia:")) != -1) {
    switch (opt) {
    case 'i': {
      if (create_new_todo_db() < 0) {
        exit(EXIT_FAILURE);
      }

      printf("Created .todo.db file at home directory...\n");
      break;
    }
    default: {
      print_usage(argv);
      exit(EXIT_FAILURE);
    }
    }
  }

  return 0;
}
