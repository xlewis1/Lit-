#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define MAX_PATH 1024
#define MAX_CONTENT (1024 * 1024)

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_write_object(const char *content, size_t len, char *out_hash_hex);
int engine_get_head(char *buf, size_t max_len);
int engine_update_ref(const char *ref_name, const char *commit_hash);

int should_ignore_save(const char *name) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 1;
    if (strcmp(name, ".lit") == 0) return 1;
    if (strcmp(name, ".DS_Store") == 0) return 1;

    FILE *f = fopen(".litignore", "r");
    if (f != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), f) != NULL) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0 && line[0] != '#') {
                if (strcmp(name, line) == 0) {
                    fclose(f);
                    return 1;
                }
                if (line[0] == '*' && strstr(name, line + 1) != NULL) {
                    fclose(f);
                    return 1;
                }
            }
        }
        fclose(f);
    }
    return 0;
}

char *read_file_to_string(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len < 0 || len > MAX_CONTENT) {
        fclose(f);
        return NULL;
    }

    char *buf = malloc(len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t read_bytes = fread(buf, 1, len, f);
    buf[read_bytes] = '\0';
    *out_len = read_bytes;

    fclose(f);
    return buf;
}

void save_directory_recursive(const char *dir_path, FILE *commit_file, int *saved_count) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (should_ignore_save(entry->d_name)) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                save_directory_recursive(full_path, commit_file, saved_count);
            } else if (S_ISREG(st.st_mode)) {
                size_t file_len = 0;
                char *file_data = read_file_to_string(full_path, &file_len);
                if (file_data) {
                    char obj_hash[65];
                    if (engine_write_object(file_data, file_len, obj_hash) == 0) {
                        fprintf(commit_file, "file %s %s\n", full_path + 2, obj_hash);
                        printf("  📦 Saved: %s\n", full_path + 2);
                        (*saved_count)++;
                    }
                    free(file_data);
                }
            }
        }
    }
    closedir(dir);
}

void cmd_save(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    const char *message = "Automatic save snapshot";
    if (argc > 0 && argv[0] != NULL) {
        message = argv[0];
    }

    char manifest_path[MAX_PATH];
    snprintf(manifest_path, sizeof(manifest_path), ".lit/objects/temp_commit_%ld", (long)time(NULL));
    
    FILE *commit_f = fopen(manifest_path, "w");
    if (!commit_f) {
        printf("❌ Failed to create commit record.\n");
        return;
    }

    time_t now = time(NULL);
    fprintf(commit_f, "timestamp %ld\n", (long)now);
    fprintf(commit_f, "message %s\n", message);

    printf("🔥 Creating snapshot...\n");
    int saved_count = 0;
    save_directory_recursive(".", commit_f, &saved_count);
    fclose(commit_f);

    if (saved_count == 0) {
        printf("⚠️ No files found to save.\n");
        unlink(manifest_path);
        return;
    }

    size_t manifest_len = 0;
    char *manifest_data = read_file_to_string(manifest_path, &manifest_len);
    unlink(manifest_path);

    if (manifest_data) {
        char commit_hash[65];
        engine_write_object(manifest_data, manifest_len, commit_hash);
        free(manifest_data);

        char head_ref[256];
        if (engine_get_head(head_ref, sizeof(head_ref)) == 0) {
            engine_update_ref(head_ref, commit_hash);
            printf("✨ Successfully saved! Commit ID: %.12s\n", commit_hash);
            printf("💬 Message: \"%s\"\n", message);
        }
    }
}