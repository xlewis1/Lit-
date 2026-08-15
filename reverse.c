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

void cmd_reverse(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    if (argc == 0 || argv[0] == NULL) {
        printf("❌ Missing target commit hash. Usage: lit reverse <commit_hash>\n");
        return;
    }

    const char *target_commit = argv[0];

    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) != 0) {
        printf("❌ Failed to resolve current HEAD reference.\n");
        return;
    }

    // Update the active reference (branch or HEAD) to point back to the target commit
    if (engine_update_ref(head_ref, target_commit) == 0) {
        printf("⏪ Successfully reversed HEAD to commit %.12s\n", target_commit);
    } else {
        printf("❌ Failed to reverse: commit hash '%s' could not be found or updated.\n", target_commit);
    }
}