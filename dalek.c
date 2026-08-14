#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024

// Forward declaration from engine.c
int engine_is_initialized(void);

// Helper to match targets for total extermination
int is_target_for_extermination(const char *filename) {
    if (strstr(filename, ".env") != NULL) return 1;
    if (strstr(filename, "id_rsa") != NULL) return 1;
    if (strstr(filename, "secret") != NULL) return 1;
    if (strstr(filename, "key") != NULL) return 1;
    if (strstr(filename, "token") != NULL) return 1;
    return 0;
}

// Recursive workspace sweep to exterminate sensitive files
void dalek_sweep_directory(const char *dir_path, int *exterminated_count) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (strcmp(entry->d_name, ".lit") == 0) continue; // Keep engine intact

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                dalek_sweep_directory(full_path, exterminated_count);
            } else if (S_ISREG(st.st_mode)) {
                if (is_target_for_extermination(entry->d_name)) {
                    if (unlink(full_path) == 0) {
                        printf("🔴 [EXTERMINATED] %s\n", full_path + 2);
                        (*exterminated_count)++;
                    }
                }
            }
        }
    }
    closedir(dir);
}

void cmd_dalek(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    printf("🤖 DALEK PROTOCOL ENGAGED: Scanning workspace for sensitive files (.env, keys, secrets)...\n");
    printf("--------------------------------------------------\n");

    int count = 0;
    dalek_sweep_directory(".", &count);

    printf("--------------------------------------------------\n");
    if (count > 0) {
        printf("💥 Extermination complete! Purged %d sensitive file(s) from the workspace.\n", count);
    } else {
        printf("✨ No threats or sensitive targets found. Workspace is clean.\n");
    }
}