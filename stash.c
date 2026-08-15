#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_PATH 1024
#define STASH_DIR ".lit/stash"

// Forward declarations from engine.c
int engine_is_initialized(void);

// Helper to ensure the stash directory exists
static int ensure_stash_dir(void) {
    struct stat st;
    if (stat(STASH_DIR, &st) != 0) {
        if (mkdir(STASH_DIR, 0755) != 0) {
            return -1;
        }
    }
    return 0;
}

// Helper to copy a file from src to dest
static int copy_file(const char *src, const char *dest) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;

    FILE *out = fopen(dest, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }

    fclose(in);
    fclose(out);
    return 0;
}

// Save active workspace files into a timestamped stash directory
static void stash_push(void) {
    if (ensure_stash_dir() != 0) {
        printf("❌ Failed to initialize stash directory.\n");
        return;
    }

    time_t now = time(NULL);
    char stash_id[64];
    snprintf(stash_id, sizeof(stash_id), "stash_%ld", (long)now);

    char target_stash_path[MAX_PATH];
    snprintf(target_stash_path, sizeof(target_stash_path), "%s/%s", STASH_DIR, stash_id);

    if (mkdir(target_stash_path, 0755) != 0) {
        printf("❌ Failed to create stash state target.\n");
        return;
    }

    // Copy regular files from workspace root into stash snapshot
    DIR *dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.' || strcmp(entry->d_name, "lit") == 0) continue;

            struct stat st;
            if (stat(entry->d_name, &st) == 0 && S_ISREG(st.st_mode)) {
                char dest_file[MAX_PATH];
                snprintf(dest_file, sizeof(dest_file), "%s/%s", target_stash_path, entry->d_name);
                copy_file(entry->d_name, dest_file);
            }
        }
        closedir(dir);
    }

    // Save metadata marker
    char meta_path[MAX_PATH];
    snprintf(meta_path, sizeof(meta_path), "%s/meta.txt", target_stash_path);
    FILE *meta = fopen(meta_path, "w");
    if (meta) {
        fprintf(meta, "Stashed changes saved at timestamp %ld\n", (long)now);
        fclose(meta);
    }

    printf("📦 Saved workspace files to %s\n", stash_id);
    printf("✨ Working directory stashed cleanly.\n");
}

// Restore stashed files from the latest stash back to the working directory
static void stash_pop(void) {
    DIR *dir = opendir(STASH_DIR);
    if (!dir) {
        printf("🌿 No stashed states available to pop.\n");
        return;
    }

    struct dirent *entry;
    char latest_stash[MAX_PATH] = {0};

    // Find latest stash snapshot entry
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(latest_stash, sizeof(latest_stash), "%s/%s", STASH_DIR, entry->d_name);
    }
    closedir(dir);

    if (strlen(latest_stash) == 0) {
        printf("🌿 Stash is empty.\n");
        return;
    }

    DIR *sdir = opendir(latest_stash);
    if (sdir) {
        struct dirent *sentry;
        while ((sentry = readdir(sdir)) != NULL) {
            if (strcmp(sentry->d_name, ".") == 0 || strcmp(sentry->d_name, "..") == 0 || strcmp(sentry->d_name, "meta.txt") == 0) {
                continue;
            }

            char src_file[MAX_PATH];
            snprintf(src_file, sizeof(src_file), "%s/%s", latest_stash, sentry->d_name);

            if (copy_file(src_file, sentry->d_name) == 0) {
                printf("🔄 Restored %s\n", sentry->d_name);
            }
        }
        closedir(sdir);
    }

    printf("✨ Successfully popped latest stashed state into working tree!\n");
}

// List all currently stashed working directory states
static void stash_list(void) {
    DIR *dir = opendir(STASH_DIR);
    if (!dir) {
        printf("🌿 No stashed states found.\n");
        return;
    }

    printf("📦 Stashed Workspace States:\n");
    printf("----------------------------------------\n");

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char meta_path[MAX_PATH];
        snprintf(meta_path, sizeof(meta_path), "%s/%s/meta.txt", STASH_DIR, entry->d_name);

        FILE *f = fopen(meta_path, "r");
        char desc[256] = "No metadata available";
        if (f) {
            if (fgets(desc, sizeof(desc), f)) {
                desc[strcspn(desc, "\r\n")] = 0;
            }
            fclose(f);
        }

        printf("  * [%s] - %s\n", entry->d_name, desc);
        count++;
    }
    closedir(dir);

    if (count == 0) {
        printf("🌿 No stashed states found.\n");
    }
    printf("----------------------------------------\n");
}

void cmd_stash(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    if (argc == 0 || argv[0] == NULL) {
        stash_push();
        return;
    }

    const char *subcommand = argv[0];

    if (strcmp(subcommand, "list") == 0) {
        stash_list();
    } 
    else if (strcmp(subcommand, "pop") == 0) {
        stash_pop();
    }
    else if (strcmp(subcommand, "save") == 0 || strcmp(subcommand, "push") == 0) {
        stash_push();
    } 
    else {
        printf("❌ Unknown stash action '%s'. Use `lit stash`, `lit stash pop`, or `lit stash list`.\n", subcommand);
    }
}