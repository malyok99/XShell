#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "todo.h"
#include <ctype.h>

#define RED "\033[0;31m"
#define RESET "\033[0m"
#define GREEN "\033[1;32m"
#define MAX_TASKS 100
#define TASK_LEN 256
#define FILE_NAME "tasks.txt"

char tasks[MAX_TASKS][TASK_LEN];
int task_count = 0;

void load_tasks() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) return;

    while (fgets(tasks[task_count], TASK_LEN, file)) {
        tasks[task_count][strcspn(tasks[task_count], "\n")] = '\0';
        task_count++;
    }

    fclose(file);
}

void save_tasks() {
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        perror("Failed to save tasks\n");
        return;
    }

    for (int i = 0; i < task_count; i++) {
        fprintf(file, "%s\n", tasks[i]);
    }

    fclose(file);
}

void view_tasks() {
    if (task_count == 0) {
        printf("Empty.\n");
        return;
    }

    printf("ToDo List:\n");
    for (int i = 0; i < task_count; i++) {
        printf("%d. %s\n", i + 1, tasks[i]);
    }
}

void add_task(char *task) {
    if (task_count >= MAX_TASKS) {
        printf("%sToDo List is full.%s\n", RED, RESET);
        return;
    }

    strncpy(tasks[task_count], task, TASK_LEN - 1);
    tasks[task_count][TASK_LEN - 1] = '\0';
    task_count++;

    save_tasks();
    printf("%sTask added successfully.%s\n", GREEN, RESET);
}

void delete_task(char *input) {
    if (task_count == 0) {
        printf("No tasks to delete.\n");
        return;
    }

    int index = -1;

    if (isdigit(input[0])) {
        index = atoi(input) - 1;
        if (index < 0 || index >= task_count) {
            printf("%sTask ID is out of range.%s\n", RED, RESET);
            return;
        }
        printf("%sTask with ID %s deleted successfully.%s\n", GREEN, input, RESET);
    } else {
        for (int i = 0; i < task_count; i++) {
            if (strcmp(tasks[i], input) == 0) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            printf("%sTask not found.%s\n", RED, RESET);
            return;
        }
        printf("%sTask '%s' deleted successfully.%s\n", GREEN, input, RESET);
    }

    for (int i = index; i < task_count - 1; i++) {
        strcpy(tasks[i], tasks[i + 1]);
    }

    task_count--;
    save_tasks();
}
