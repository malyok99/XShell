#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#include "todo.h"

#define RED "\033[0;31m"
#define RESET "\033[0m"

int xshell_cd(char **args) {
    char *dir = args[1] ? args[1] : getenv("HOME");
    if (!dir) {
        fprintf(stderr, "%sXshell: HOME environment variable not set%s\n", RED, RESET);
        return 1;
    }
    if (chdir(dir) != 0) perror("Xshell");
    return 1;
}

int xshell_todo(char **args) {
    if (!args[1]) {
        view_tasks();
    } else if (strcmp(args[1], "-a") == 0) {
        if (!args[2]) printf("ToDo: Task name is missing.\n");
        else add_task(args[2]);
    } else if (strcmp(args[1], "-r") == 0) {
        if (!args[2]) printf("ToDo: Task ID or name is missing.\n");
        else delete_task(args[2]);
    } else {
        printf("ToDo: Unknown option '%s'.\n", args[1]);
    }
    return 1;
}

int xshell_exit(char **args) {
    return 0;
}
