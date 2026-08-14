#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_PATH 1024

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);

void cmd_checkout(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    if (argc == 0 || argv[0] == NULL) {
        printf("❌ Missing target branch or commit ID. Usage: lit checkout <branch_name_or_commit_id>\n");
        return;
    }

    const char *target = argv[0];
    char target_commit[65] = {0};

    // Check if target is an existing branch name
    char branch_path[MAX_PATH];
    snprintf(branch_path, sizeof(branch_path), ".lit/refs/heads/%s", target);

    struct stat st;
    if (stat(branch_path, &st) == 0) {
        // It's a branch! Update HEAD to point to this branch reference
        FILE *f = fopen(branch_path, "r");
        if (f) {
            fscanf(f, "%64s", target_commit);
            fclose(f);
        }

        FILE *head_file = fopen(".lit/HEAD", "w");
        if (head_file) {
            fprintf(head_file, "ref: refs/heads/%s\n", target);
            fclose(head_file);
        }

        printf("🚀 Switched to branch '%s' (pointing to commit %.12s)\n", target, target_commit);
    } else {
        // Assume target is a direct commit ID hash
        snprintf(target_commit, sizeof(target_commit), "%s", target);
        
        char commit_path[MAX_PATH];
        snprintf(commit_path, sizeof(commit_path), ".lit/objects/%s", target_commit);

        if (stat(commit_path, &st) != 0) {
            printf("❌ Target '%s' is neither a valid branch nor an existing commit ID.\n", target);
            return;
        }

        // Detach HEAD and point directly to the commit hash
        FILE *head_file = fopen(".lit/HEAD", "w");
        if (head_file) {
            fprintf(head_file, "%s\n", target_commit);
            fclose(head_file);
        }

        printf("🧭 Checked out commit directly (detached HEAD): %.12s\n", target_commit);
    }

    printf("✨ Working tree reference updated successfully.\n");
}