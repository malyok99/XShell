#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <pwd.h>
#include <time.h>

#include "todo.h"
#include "xshell_commands.h"
#include "extra.h"

#define TOK_DELIM " \t\r\n"
#define RESET "\033[0m"
#define TK_BUFF_SIZE 64
#define HISTORY_SIZE 100

void loop();
char *read_line();
char **split_line(char *);
int command_execute(char **args);
void add_to_history(const char *line);

char *history[HISTORY_SIZE];
int history_count = 0;
int history_position = -1;

char hostname[256];
char *username;

int main() {
    gethostname(hostname, sizeof(hostname));
    username = getenv("USER");

    create_xshellrc();
    load_config();
    load_tasks();
    loop();

    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}

void loop() {
    int status = 1;
    do {
        char *line = read_line();
        add_to_history(line);
        char **args = split_line(line);
        status = command_execute(args);
        free(line);
        free(args);
    } while (status);
}

char *read_line() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int buffsize = 1024, position = 0;
    char *buffer = malloc(sizeof(char) * buffsize);
    char cwd[1024];
    struct passwd *pw = getpwuid(getuid());
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (!buffer) {
        fprintf(stderr, "%sXshell: Allocation error%s\n", error_color, RESET);
        exit(EXIT_FAILURE);
    }

    getcwd(cwd, sizeof(cwd));
    modify_cwd(cwd, sizeof(cwd));

    char *custom_prompt = replace_placeholders(
        prompt_format,
        username, hostname, cwd,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        RESET
    );
    printf("%s", custom_prompt);
    free(custom_prompt);

    int c;

    while (1) {
        c = getchar();

        if (c == '\n') {
            buffer[position] = '\0';
            printf("\n");
            break;
        } else if (c == 27) {
            getchar();
            c = getchar();

            if (c == 'A' || c == 'B') { // Up/Down arrow keys
                if (history_count > 0) {
                    history_position = (history_position + (c == 'A' ? -1 : (c == 'B' ? 1 : 0)) + history_count) % history_count;

                    strcpy(buffer, history[history_position]);
                    position = strlen(buffer);

                    char *custom_secondary = replace_placeholders(
                        secondary_format,
                        username, hostname, cwd,
                        tm.tm_hour, tm.tm_min, tm.tm_sec, RESET
                    );

                    printf("%s%s", custom_secondary, buffer);
                    free(custom_secondary);
                }
            }
        } else if (c == 127) { // Backspace
            if (position > 0) {
                position--;
                buffer[position] = '\0';

                char *custom_secondary = replace_placeholders(
                        secondary_format,
                        username, hostname, cwd,
                        tm.tm_hour, tm.tm_min, tm.tm_sec, RESET
                    );

                char format_with_clear[512];
                snprintf(format_with_clear, sizeof(format_with_clear), "%s%s %s", custom_secondary,buffer, "\033[K");
                printf(format_with_clear, time_color, tm.tm_hour, tm.tm_min, tm.tm_sec, RESET, buffer);

                free(custom_secondary);

                printf("\033[D");
            }
        } else {
            buffer[position++] = c;
            buffer[position] = '\0';

            char *custom_secondary = replace_placeholders(
                secondary_format,
                username, hostname, cwd,
                tm.tm_hour, tm.tm_min, tm.tm_sec, RESET
            );
            printf("%s%s", custom_secondary,buffer);
            free(custom_secondary);
        }

        if (position >= buffsize) {
            buffsize += 1024;
            char *temp = realloc(buffer, buffsize);
            if (!temp) {
                fprintf(stderr, "%sXshell: Allocation error%s\n", error_color, RESET);
                exit(EXIT_FAILURE);
            }
            buffer = temp;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return buffer;
}

char **split_line(char *line) {
    int buffsize = TK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "%sXshell: Allocation error%s\n", error_color, RESET);
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position++] = token;

        if (position >= buffsize) {
            buffsize += TK_BUFF_SIZE;
            char **temp = realloc(tokens, buffsize * sizeof(char *));
            if (!temp) {
                fprintf(stderr, "%sXshell: Allocation error%s\n", error_color, RESET);
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = temp;
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;

    return tokens;
}

int command_execute(char **args) {
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) return xshell_exit(args);
    if (strcmp(args[0], "cd") == 0) return xshell_cd(args);
    if (strcmp(args[0], "todo") == 0) return xshell_todo(args);

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            printf("%sXshell: command not found: %s%s\n", error_color, args[0], RESET);
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Xshell");
    } else {
        wait(NULL);
    }
    return 1;
}

void add_to_history(const char *line) {
    if (history_count < HISTORY_SIZE) {
        history[history_count++] = strdup(line);
    } else {
        free(history[0]);
        memmove(&history[0], &history[1], (HISTORY_SIZE - 1) * sizeof(char *));
        history[HISTORY_SIZE - 1] = strdup(line);
    }
}