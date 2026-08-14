#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);
int engine_update_ref(const char *ref_name, const char *commit_hash);

void cmd_ignite(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    // Default remote source directory or URL path passed as argument
    const char *remote_source = ".lit_remote";
    if (argc > 0 && argv[0] != NULL) {
        remote_source = argv[0];
    }

    printf("🔥 Igniting engine... pulling updates from: %s\n", remote_source);

    // Verify remote source exists
    struct stat st;
    if (stat(remote_source, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("⚠️ Remote source '%s' does not exist or is unreachable.\n", remote_source);
        return;
    }

    // Read current local HEAD reference
    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) != 0) {
        printf("❌ Failed to read local HEAD pointer.\n");
        return;
    }

    // Locate the remote branch reference file (e.g., refs/heads/main)
    char remote_ref_path[MAX_PATH];
    if (strncmp(head_ref, "ref: ", 5) == 0) {
        snprintf(remote_ref_path, sizeof(remote_ref_path), "%s/%s", remote_source, head_ref + 5);
    } else {
        snprintf(remote_ref_path, sizeof(remote_ref_path), "%s/refs/heads/main", remote_source);
    }

    FILE *f = fopen(remote_ref_path, "r");
    if (!f) {
        printf("⚠️ Could not read reference from remote repository.\n");
        return;
    }

    char remote_commit_hash[65];
    if (fscanf(f, "%64s", remote_commit_hash) != 1) {
        fclose(f);
        printf("⚠️ Remote reference is empty.\n");
        return;
    }
    fclose(f);

    // Update local reference to match the pulled remote commit hash (fast-forward integration)
    if (engine_update_ref(head_ref, remote_commit_hash) == 0) {
        printf("🚀 Successfully ignited and pulled changes!\n");
        printf("✨ Local branch updated to remote commit: %.12s\n", remote_commit_hash);
    } else {
        printf("❌ Failed to integrate remote updates into local workspace.\n");
    }
}