#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_PATH 1024

// Simple utility to check if a directory exists
int engine_is_initialized(void) {
    struct stat st;
    return (stat(".lit", &st) == 0 && S_ISDIR(st.st_mode));
}

// Initialize the .lit workspace directory structure
int engine_init(void) {
    if (engine_is_initialized()) {
        return 0; // Already initialized
    }

    if (mkdir(".lit", 0755) != 0) return -1;
    if (mkdir(".lit/objects", 0755) != 0) return -1;
    if (mkdir(".lit/refs", 0755) != 0) return -1;
    if (mkdir(".lit/refs/heads", 0755) != 0) return -1;

    // Create default HEAD pointer pointing to main branch
    FILE *f = fopen(".lit/HEAD", "w");
    if (!f) return -1;
    fprintf(f, "ref: refs/heads/main\n");
    fclose(f);

    // Create empty main branch file
    FILE *main_ref = fopen(".lit/refs/heads/main", "w");
    if (main_ref) {
        fclose(main_ref);
    }

    return 0;
}

// Get the current active branch or reference from HEAD
int engine_get_head(char *buf, size_t max_len) {
    FILE *f = fopen(".lit/HEAD", "r");
    if (!f) return -1;
    
    if (fgets(buf, max_len, f) != NULL) {
        // Strip trailing newline
        buf[strcspn(buf, "\r\n")] = 0;
        fclose(f);
        return 0;
    }
    
    fclose(f);
    return -1;
}

// Simple custom hash function to create unique object filenames from data (content-addressable)
void engine_hash_data(const char *data, size_t len, char *out_hash_hex) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)data[i];
    }
    sprintf(out_hash_hex, "%016lx%016lx", hash, hash ^ 0xFFFFFFFFFFFFFFFFUL);
}

// Write an object into .lit/objects based on its content hash
int engine_write_object(const char *content, size_t len, char *out_hash_hex) {
    engine_hash_data(content, len, out_hash_hex);

    char path[MAX_PATH];
    snprintf(path, sizeof(path), ".lit/objects/%s", out_hash_hex);

    // If object already exists, we don't need to rewrite it (content-addressable deduplication)
    if (access(path, F_OK) == 0) {
        return 0;
    }

    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fwrite(content, 1, len, f);
    fclose(f);

    return 0;
}

// Update a reference (like refs/heads/main) with a new commit hash
int engine_update_ref(const char *ref_name, const char *commit_hash) {
    char path[MAX_PATH];
    // If ref_name starts with 'ref: ', strip it for direct file path writing
    const char *target = ref_name;
    if (strncmp(ref_name, "ref: ", 5) == 0) {
        target = ref_name + 5;
    }

    snprintf(path, sizeof(path), ".lit/%s", target);

    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "%s\n", commit_hash);
    fclose(f);

    return 0;
}