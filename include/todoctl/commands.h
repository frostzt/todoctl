/*
 * commands.h -- TodoCtl Commands
 *
 * Author: frostzt
 * Date: 2026-01-26
 */

#ifndef TODOCTL_COMMANDS_H
#define TODOCTL_COMMANDS_H

#include <stddef.h>
#include <stdint.h>

/* adds a task into db */
int add_task_command(const char *);

/* list all the tasks available */
int list_tasks_command(int);

/* marks a task done */
int mark_task_done(const uint64_t id);

#endif // TODOCTL_COMMANDS_H
