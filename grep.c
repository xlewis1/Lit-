#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 2048

typedef struct {
    bool ignore_case;
    bool show_line_numbers;
    char *pattern;
} GrepOptions;

void to_lower_str(char *dest, const char *src) {
    while (*src) {
        *dest = (char)tolower((unsigned char)*src);
        dest++;
        src++;
    }
    *dest = '\0';
}

void search_file(const char *filepath, const GrepOptions *opts) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        return; 
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 1;
    char search_pattern[MAX_LINE_LENGTH];
    
    if (opts->ignore_case) {
        to_lower_str(search_pattern, opts->pattern);
    } else {
        strcpy(search_pattern, opts->pattern);
    }

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        char check_line[MAX_LINE_LENGTH];
        if (opts->ignore_case) {
            to_lower_str(check_line, line);
        } else {
            strcpy(check_line, line);
        }

        if (strstr(check_line, search_pattern) != NULL) {
            if (opts->show_line_numbers) {
                printf("%s:%d:%s\n", filepath, line_number, line);
            } else {
                printf("%s:%s\n", filepath, line);
            }
        }
        line_number++;
    }

    fclose(file);
}

void search_directory(const char *dir_path, const GrepOptions *opts) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' || 
            strcmp(entry->d_name, ".lit") == 0) {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            search_directory(full_path, opts);
        } else if (entry->d_type == DT_REG) {
            search_file(full_path, opts);
        }
    }

    closedir(dir);
}

void cmd_grep(int argc, char *argv[]) {
    GrepOptions opts = {
        .ignore_case = false,
        .show_line_numbers = true,
        .pattern = NULL
    };

    int i = 0;
    while (i < argc) {
        if (strcmp(argv[i], "-i") == 0) {
            opts.ignore_case = true;
        } else if (strcmp(argv[i], "-n") == 0) {
            opts.show_line_numbers = true;
        } else if (strcmp(argv[i], "-N") == 0) {
            opts.show_line_numbers = false;
        } else {
            opts.pattern = argv[i];
            break;
        }
        i++;
    }

    if (!opts.pattern) {
        fprintf(stderr, "Error: No search pattern specified.\n");
        fprintf(stderr, "Usage: lit grep [options] <pattern>\n");
        return;
    }

    DIR *lit_dir = opendir(".lit");
    if (!lit_dir) {
        fprintf(stderr, "fatal: not a lit repository (or any of the parent directories): .lit\n");
        return;
    }
    closedir(lit_dir);

    search_directory(".", &opts);
}