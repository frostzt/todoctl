/*
 * commands.h -- TodoCtl Commands
 *
 * Author: frostzt
 * Date: 2026-01-26
 */

#ifndef TODOCTL_COMMANDS_H
#define TODOCTL_COMMANDS_H

#include <stddef.h>

/* adds a task into db */
int add_task_command(const char *);

/* list all the tasks available */
int list_tasks_command(const size_t limit);

#endif // TODOCTL_COMMANDS_H
