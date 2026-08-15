#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>

#define MAX_PATH 1024
#define MAX_PATTERNS 256
#define MAX_PATTERN_LEN 128
#define MAX_INDEX_ENTRIES 2048

int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);

typedef struct {
    char patterns[MAX_PATTERNS][MAX_PATTERN_LEN];
    int count;
} IgnoreRules;

typedef struct {
    char filepath[MAX_PATH];
} IndexEntry;

static IgnoreRules global_rules;
static IndexEntry index_entries[MAX_INDEX_ENTRIES];
static int index_count = 0;

void load_ignore_rules(const char *filepath) {
    global_rules.count = 0;
    strncpy(global_rules.patterns[global_rules.count++], ".lit", MAX_PATTERN_LEN);
    strncpy(global_rules.patterns[global_rules.count++], ".git", MAX_PATTERN_LEN);

    FILE *f = fopen(filepath, "r");
    if (!f) return;

    char line[MAX_PATTERN_LEN];
    while (fgets(line, sizeof(line), f) && global_rules.count < MAX_PATTERNS) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0' || line[0] == '#') continue;
        strncpy(global_rules.patterns[global_rules.count++], line, MAX_PATTERN_LEN);
    }
    fclose(f);
}

int should_ignore(const char *name) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 1;

    for (int i = 0; i < global_rules.count; i++) {
        const char *pattern = global_rules.patterns[i];
        if (strcmp(name, pattern) == 0) return 1;
        if (fnmatch(pattern, name, FNM_PATHNAME) == 0) return 1;
    }
    return 0;
}

void add_tracked_entry(const char *filepath) {
    for (int i = 0; i < index_count; i++) {
        if (strcmp(index_entries[i].filepath, filepath) == 0) return;
    }
    if (index_count < MAX_INDEX_ENTRIES) {
        strncpy(index_entries[index_count++].filepath, filepath, MAX_PATH);
    }
}

void load_from_commit(const char *commit_hash) {
    if (strlen(commit_hash) == 0) return;

    char object_path[MAX_PATH];
    snprintf(object_path, sizeof(object_path), ".lit/objects/%s", commit_hash);

    FILE *f = fopen(object_path, "r");
    if (!f) return;

    char line[MAX_PATH + 128];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(line, "file ", 5) == 0) {
            char path[MAX_PATH];
            char hash[65];
            if (sscanf(line + 5, "%s %s", path, hash) >= 1) {
                add_tracked_entry(path);
            }
        }
    }
    fclose(f);
}

void load_index(void) {
    index_count = 0;

    // 1. Read .lit/index if present
    FILE *f = fopen(".lit/index", "r");
    if (f) {
        char line[MAX_PATH + 64];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            char *path_ptr = strchr(line, ' ');
            if (path_ptr) path_ptr++;
            else path_ptr = line;
            if (strlen(path_ptr) > 0) {
                add_tracked_entry(path_ptr);
            }
        }
        fclose(f);
    }

    // 2. Read current HEAD commit manifest from .lit/objects/
    char head_buf[256];
    if (engine_get_head(head_buf, sizeof(head_buf)) == 0) {
        char commit_hash[65] = {0};
        if (strncmp(head_buf, "ref: ", 5) == 0) {
            char ref_path[MAX_PATH];
            snprintf(ref_path, sizeof(ref_path), ".lit/%s", head_buf + 5);
            FILE *f_ref = fopen(ref_path, "r");
            if (f_ref) {
                if (fgets(commit_hash, sizeof(commit_hash), f_ref)) {
                    commit_hash[strcspn(commit_hash, "\r\n")] = 0;
                }
                fclose(f_ref);
            }
        } else {
            strncpy(commit_hash, head_buf, sizeof(commit_hash));
        }
        load_from_commit(commit_hash);
    }
}

int is_tracked(const char *filepath) {
    for (int i = 0; i < index_count; i++) {
        if (strcmp(index_entries[i].filepath, filepath) == 0) {
            return 1;
        }
    }
    return 0;
}

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
                scan_directory(full_path, untracked_count);
            } else if (S_ISREG(st.st_mode)) {
                const char *display_path = (strncmp(full_path, "./", 2) == 0) ? full_path + 2 : full_path;
                
                if (!is_tracked(display_path)) {
                    printf("  ✨ (untracked)  %s\n", display_path);
                    (*untracked_count)++;
                }
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

    load_ignore_rules(".litignore");
    load_index();

    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) == 0) {
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