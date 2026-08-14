#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 1024

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);

// Helper to check if a file/path should be ignored (e.g., .lit folder, object files, etc.)
int should_ignore(const char *name) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 1;
    if (strcmp(name, ".lit") == 0) return 1;
    if (strcmp(name, ".ccm") == 0) return 1; // if your build system uses this
    // Ignore compiled objects or binary outputs if needed
    if (strstr(name, ".o") != NULL && name[strlen(name) - 2] == '.' && name[strlen(name) - 1] == 'o') return 1;
    return 0;
}

// Recursive status scan function
void scan_directory(const char *dir_path, int *untracked_count) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (should_ignore(entry->d_name)) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recurse into subdirectories (skipping the prefix './' for readability)
                scan_directory(full_path, untracked_count);
            } else if (S_ISREG(st.st_mode)) {
                // Print untracked file cleanly
                printf("  ✨ (untracked)  %s\n", full_path + 2); // strip leading './'
                (*untracked_count)++;
            }
        }
    }
    closedir(dir);
}

void cmd_status(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) == 0) {
        // Clean, human-friendly branch display
        if (strncmp(head_ref, "ref: refs/heads/", 16) == 0) {
            printf("🔥 On branch: %s\n", head_ref + 16);
        } else {
            printf("🔥 HEAD detached at: %s\n", head_ref);
        }
    }

    printf("----------------------------------------\n");
    printf("📁 Working Tree Status:\n");

    int untracked_count = 0;
    scan_directory(".", &untracked_count);

    if (untracked_count == 0) {
        printf("  🌿 Working tree clean. Nothing new to save.\n");
    }

    printf("----------------------------------------\n");
}