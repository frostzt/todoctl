#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "todoctl/commands.h"
#include "todoctl/db.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-a <task>] [-i]\n", argv[0]);
  printf("\t -i initialize todoctl\n");
  printf("\t -a adds a new task\n");
  printf("\t -l list all the tasks\n");
}

int main(int argc, char *argv[]) {
  int opt;

  /* parse flags right now `init` is a flag and does not take
   * an argument will have to think on how to approach this */
  while ((opt = getopt(argc, argv, "lia:")) != -1) {
    switch (opt) {
    /* TODO: Right now init via flag; need a command like `todoctl init` */
    case 'i': {
      if (create_new_todo_db() < 0) { exit(EXIT_FAILURE); }
      printf("Created .todo.db file at home directory...\n");
      break;
    }

    /* Add a new task */
    case 'a': {
      const char *task = optarg;
      if (add_task_command(task) < 0) {
        fprintf(stderr, "Failed to add task!");
        exit(EXIT_FAILURE);
      }
      break;
    }

    /* list all the tasks */
    case 'l': {
      // TODO: Handle limits
      if (list_tasks_command(1) < 0) {
        fprintf(stderr, "Failed to list tasks!");
        exit(EXIT_FAILURE);
      }
      break;
    }

    case '?': {
      print_usage(argv);
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
