#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH 1024

// Forward declaration from engine.c
int engine_is_initialized(void);

// Recursive helper to remove a directory and all of its contents (.lit internal database)
int recursive_remove(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) return -1;

    struct dirent *entry;
    int success = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                recursive_remove(path);
            } else {
                unlink(path);
            }
        }
    }
    closedir(dir);
    
    if (rmdir(dir_path) != 0) {
        success = -1;
    }
    return success;
}

void cmd_nuke(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (!engine_is_initialized()) {
        printf("⚠️ No active .lit repository found to nuke.\n");
        return;
    }

    printf("💥 Nuke sequence initiated. Destroying .lit version history and database...\n");

    if (recursive_remove(".lit") == 0) {
        printf("☢️ Repository successfully nuked! All tracked history has been erased.\n");
    } else {
        printf("❌ Failed to completely remove the .lit workspace.\n");
    }
}