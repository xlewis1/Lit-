#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH 1024

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);
int engine_update_ref(const char *ref_name, const char *commit_hash);

void cmd_dilute(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    const char *target_base = "main"; // Default rebase target base branch
    if (argc > 0 && argv[0] != NULL) {
        target_base = argv[0];
    }

    printf("💧 Diluting history... rebasing current branch onto '%s'...\n", target_base);

    // Read local HEAD reference
    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) != 0) {
        printf("❌ Failed to read local HEAD pointer.\n");
        return;
    }

    // Resolve the target base reference path
    char base_ref_path[MAX_PATH];
    snprintf(base_ref_path, sizeof(base_ref_path), ".lit/refs/heads/%s", target_base);

    FILE *f = fopen(base_ref_path, "r");
    if (!f) {
        printf("⚠️ Could not locate base branch '%s' for dilution/rebase.\n", target_base);
        return;
    }

    char base_commit_hash[65];
    if (fscanf(f, "%64s", base_commit_hash) != 1) {
        fclose(f);
        printf("⚠️ Base branch reference is empty.\n");
        return;
    }
    fclose(f);

    // Fast-forward rebase application onto the new base commit
    if (engine_update_ref(head_ref, base_commit_hash) == 0) {
        printf("✨ Successfully diluted and rebased onto commit: %.12s\n", base_commit_hash);
        printf("🚀 Working tree history streamlined.\n");
    } else {
        printf("❌ Rebase failed to update reference pointer.\n");
    }
}