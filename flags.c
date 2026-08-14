#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IGNORE_FILE ".litignore"

int handle_remote_flag(const char *action, const char *url) {
    if (!action) {
        printf("❌ Missing remote action. Usage: lit --remote <add|rm> <url>\n");
        return -1;
    }

    if (strcmp(action, "add") == 0) {
        if (!url) {
            printf("❌ Missing remote URL/host address to add.\n");
            return -1;
        }

        FILE *f = fopen(".lit/remote", "w");
        if (!f) {
            printf("❌ Failed to save remote configuration. Is a .lit repository initialized?\n");
            return -1;
        }
        fprintf(f, "%s\n", url);
        fclose(f);
        printf("🌐 Successfully registered remote target: %s\n", url);
    } 
    else if (strcmp(action, "rm") == 0) {
        if (remove(".lit/remote") == 0) {
            printf("🗑️ Successfully removed configured remote target.\n");
        } else {
            printf("⚠️ No remote target currently configured.\n");
        }
    } 
    else {
        printf("❌ Unknown remote action '%s'. Use 'add' or 'rm'.\n", action);
        return -1;
    }

    return 0;
}

int handle_ignore_flag(const char *pattern) {
    if (!pattern) {
        printf("❌ Missing pattern to ignore. Usage: lit --ignore <pattern>\n");
        return -1;
    }

    FILE *f = fopen(IGNORE_FILE, "a");
    if (!f) {
        printf("❌ Failed to open %s configuration file.\n", IGNORE_FILE);
        return -1;
    }

    fprintf(f, "%s\n", pattern);
    fclose(f);

    printf("🛡️ Added pattern '%s' to %s. Files matching this will be skipped during saves.\n", pattern, IGNORE_FILE);
    return 0;
}

// Matches the exact signature expected by main.c
int parse_flags(int argc, char *argv[]) {
    if (argc < 1 || argv == NULL || argv[0] == NULL) return 0;

    if (strcmp(argv[0], "--remote") == 0) {
        const char *action = (argc > 1) ? argv[1] : NULL;
        const char *url = (argc > 2) ? argv[2] : NULL;
        handle_remote_flag(action, url);
        return 1;
    }
    
    if (strcmp(argv[0], "--ignore") == 0) {
        const char *pattern = (argc > 1) ? argv[1] : NULL;
        handle_ignore_flag(pattern);
        return 1;
    }

    return 0;
}