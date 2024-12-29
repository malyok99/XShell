#ifndef EXTRA_H
#define EXTRA_H

extern char *success_color;
extern char *additional_color;
extern char *error_color;
extern char *time_color;
extern char *prompt_format;
extern char *secondary_format;

void create_xshellrc();
void load_config();
char *replace_placeholders(const char *format, const char *username, const char *hostname, const char *cwd, int hour, int min, int sec, const char *reset);
void modify_cwd(char *cwd, size_t size);

#endif
