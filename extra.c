#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "extra.h"

extern char *success_color, *additional_color, *error_color, *time_color, *prompt_format, *secondary_format;

void create_xshellrc() {
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/.xshellrc", getenv("HOME"));
    FILE *file = fopen(config_path, "r");

    if (file == NULL) {
        file = fopen(config_path, "w");
        if (file == NULL) {
            perror("Не удалось создать файл .xshellrc");
            return;
        }

        const char *content =
            "# ~/.xshellrc\n"
            "SUCCESS_COLOR=\\033[1;34m\n"
            "ADDITIONAL_COLOR=\\033[1;36m\n"
            "ERROR_COLOR=\\033[1;31m\n"
            "TIME_COLOR=\\033[1;33m\n"
            "PROMPT_FORMAT=\\n%{SUCCESS_COLOR}[%{USERNAME}@%{HOSTNAME}] %{ADDITIONAL_COLOR}%{CWD} \\n%{TIME_COLOR}[%{HOUR}:%{MIN}:%{SEC}] %{SUCCESS_COLOR}❯ %{RESET}\n"
            "SECONDARY_FORMAT=\\r%{TIME_COLOR}[%{HOUR}:%{MIN}:%{SEC}] %{SUCCESS_COLOR}❯ %{RESET}";

        fprintf(file, "%s", content);
        fclose(file);
    } else {
        fclose(file);
    }
}

void load_config() {
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/.xshellrc", getenv("HOME"));
    FILE *config = fopen(config_path, "r");

    char line[256];
    while (fgets(line, sizeof(line), config)) {
        char *value = strchr(line, '=');
        if (value) {
            *value = '\0';
            value++;
            while (*value == ' ' || *value == '\t') value++;

            char *end = value + strlen(value) - 1;
            while (end > value && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) {
                *end = '\0';
                end--;
            }

            char *escape_seq = "\\033";
            if (strstr(value, escape_seq)) {
                char *pos = strstr(value, escape_seq);
                while (pos) {
                    *pos = '\033';
                    memmove(pos + 1, pos + 4, strlen(pos + 4) + 1);
                    pos = strstr(pos + 1, escape_seq);
                }
            }

            char *newline_seq = "\\n";
            if (strstr(value, newline_seq)) {
                char *pos = strstr(value, newline_seq);
                while (pos) {
                    *pos = '\n';
                    memmove(pos + 1, pos + 2, strlen(pos + 2) + 1);
                    pos = strstr(pos + 1, newline_seq);
                }
            }

            char *carriage_seq = "\\r";
            if (strstr(value, carriage_seq)) {
                char *pos = strstr(value, carriage_seq);
                while (pos) {
                    *pos = '\r';
                    memmove(pos + 1, pos + 2, strlen(pos + 2) + 1);
                    pos = strstr(pos + 1, carriage_seq);
                }
            }

            if (strstr(line, "SUCCESS_COLOR")) {
                success_color = strdup(value);
            } else if (strstr(line, "ADDITIONAL_COLOR")) {
                additional_color = strdup(value);
            } else if (strstr(line, "ERROR_COLOR")) {
                error_color = strdup(value);
            } else if (strstr(line, "TIME_COLOR")) {
                time_color = strdup(value);
            } else if (strstr(line, "PROMPT_FORMAT")) {
                prompt_format = strdup(value);
            } else if (strstr(line, "SECONDARY_FORMAT")) {
                secondary_format = strdup(value);
            }
        }
    }
    fclose(config);

    if (!success_color) success_color = strdup("\033[1;34m");
    if (!additional_color) additional_color = strdup("\033[1;36m");
    if (!error_color) error_color = strdup("\033[1;31m");
    if (!time_color) time_color = strdup("\033[1;33m");
    if (!prompt_format) prompt_format = strdup("%{SUCCESS_COLOR}[%{USERNAME}@%{HOSTNAME}] %{ADDITIONAL_COLOR}%{CWD} \n%{TIME_COLOR}[%{HOUR}:%{MIN}:%{SEC}] %{SUCCESS_COLOR}❯ %{RESET}");
    if (!secondary_format) secondary_format = strdup("\r%{TIME_COLOR}[%{HOUR}:%{MIN}:%{SEC}] %{SUCCESS_COLOR}❯ %{RESET}");
}

char *replace_placeholders(const char *format, const char *username, const char *hostname, const char *cwd,
                           int hour, int min, int sec, const char *reset) {
    char *result = malloc(1024);
    if (!result) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';

    const char *ptr = format;
    while (*ptr) {
        if (*ptr == '%' && *(ptr + 1) == '{') {
            ptr += 2;
            const char *end = strchr(ptr, '}');
            if (!end) break;

            char placeholder[32];
            strncpy(placeholder, ptr, end - ptr);
            placeholder[end - ptr] = '\0';
            ptr = end + 1;

            if (strcmp(placeholder, "USERNAME") == 0) {
                strcat(result, username);
            } else if (strcmp(placeholder, "HOSTNAME") == 0) {
                strcat(result, hostname);
            } else if (strcmp(placeholder, "CWD") == 0) {
                strcat(result, cwd);
            } else if (strcmp(placeholder, "HOUR") == 0) {
                char hour_str[3];
                snprintf(hour_str, sizeof(hour_str), "%02d", hour);
                strcat(result, hour_str);
            } else if (strcmp(placeholder, "MIN") == 0) {
                char min_str[3];
                snprintf(min_str, sizeof(min_str), "%02d", min);
                strcat(result, min_str);
            } else if (strcmp(placeholder, "SEC") == 0) {
                char sec_str[3];
                snprintf(sec_str, sizeof(sec_str), "%02d", sec);
                strcat(result, sec_str);
            } else if (strcmp(placeholder, "ERROR_COLOR") == 0) {
                strcat(result, error_color);
            } else if (strcmp(placeholder, "TIME_COLOR") == 0) {
                strcat(result, time_color);
            } else if (strcmp(placeholder, "ADDITIONAL_COLOR") == 0) {
                strcat(result, additional_color);
            } else if (strcmp(placeholder, "SUCCESS_COLOR") == 0) {
                strcat(result, success_color);
            } else if (strcmp(placeholder, "RESET") == 0) {
                strcat(result, reset);
            }
        } else {
            strncat(result, ptr, 1);
            ptr++;
        }
    }

    return result;
}

void modify_cwd(char *cwd, size_t size) {
    struct passwd *pw = getpwuid(getuid());
    const char *home = pw->pw_dir;
    size_t home_len = strlen(home);

    if (strncmp(cwd, home, home_len) == 0) {
        memmove(cwd + 1, cwd + home_len, strlen(cwd) - home_len + 1);
        cwd[0] = '~';
    }
}
