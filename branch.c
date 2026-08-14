#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);
int engine_update_ref(const char *ref_name, const char *commit_hash);

// Helper to list all existing local branches
void list_branches(void) {
    DIR *dir = opendir(".lit/refs/heads");
    if (!dir) {
        printf("⚠️ Could not open branch references directory.\n");
        return;
    }

    char current_head[256];
    engine_get_head(current_head, sizeof(current_head));

    printf("🌿 Active Branches:\n");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char ref_path[256];
        snprintf(ref_path, sizeof(ref_path), "ref: refs/heads/%s", entry->d_name);

        if (strcmp(current_head, ref_path) == 0) {
            printf("  * %s (current)\n", entry->d_name);
        } else {
            printf("    %s\n", entry->d_name);
        }
    }
    closedir(dir);
}

// Helper to create a new branch pointing to the current HEAD commit
void create_branch(const char *branch_name) {
    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) != 0) {
        printf("❌ Failed to resolve current HEAD.\n");
        return;
    }

    // Resolve current HEAD commit hash
    char commit_hash[65] = {0};
    if (strncmp(head_ref, "ref: ", 5) == 0) {
        char head_path[MAX_PATH];
        snprintf(head_path, sizeof(head_path), ".lit/%s", head_ref + 5);
        FILE *f = fopen(head_path, "r");
        if (f) {
            fscanf(f, "%64s", commit_hash);
            fclose(f);
        }
    }

    if (strlen(commit_hash) == 0) {
        printf("⚠️ No commits found on current branch to branch off from.\n");
        return;
    }

    // Write new branch ref file
    char new_branch_path[MAX_PATH];
    snprintf(new_branch_path, sizeof(new_branch_path), ".lit/refs/heads/%s", branch_name);

    FILE *f = fopen(new_branch_path, "w");
    if (!f) {
        printf("❌ Failed to create branch '%s'.\n", branch_name);
        return;
    }
    fprintf(f, "%s\n", commit_hash);
    fclose(f);

    printf("✨ Created new branch '%s' at commit %.12s\n", branch_name, commit_hash);
}

// Helper to switch (checkout) an active branch
void switch_branch(const char *branch_name) {
    char branch_path[MAX_PATH];
    snprintf(branch_path, sizeof(branch_path), ".lit/refs/heads/%s", branch_name);

    struct stat st;
    if (stat(branch_path, &st) != 0) {
        printf("❌ Branch '%s' does not exist.\n", branch_name);
        return;
    }

    char new_head[256];
    snprintf(new_head, sizeof(new_head), "ref: refs/heads/%s", branch_name);

    FILE *f = fopen(".lit/HEAD", "w");
    if (!f) {
        printf("❌ Failed to update HEAD pointer.\n");
        return;
    }
    fprintf(f, "%s\n", new_head);
    fclose(f);

    printf("🚀 Switched to branch '%s'\n", branch_name);
}

// Helper to merge another branch into the current active branch
void merge_branch(const char *branch_name) {
    char source_ref_path[MAX_PATH];
    snprintf(source_ref_path, sizeof(source_ref_path), ".lit/refs/heads/%s", branch_name);

    FILE *f = fopen(source_ref_path, "r");
    if (!f) {
        printf("❌ Merge target branch '%s' does not exist.\n", branch_name);
        return;
    }

    char source_commit[65];
    if (fscanf(f, "%64s", source_commit) != 1) {
        fclose(f);
        printf("⚠️ Target branch reference is empty.\n");
        return;
    }
    fclose(f);

    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) != 0) {
        printf("❌ Failed to read current HEAD reference.\n");
        return;
    }

    // Fast-forward or merge update current branch reference to match source branch commit
    if (engine_update_ref(head_ref, source_commit) == 0) {
        printf("🔀 Successfully merged branch '%s' (Fast-forwarded to commit %.12s)\n", branch_name, source_commit);
    } else {
        printf("❌ Merge operation failed.\n");
    }
}

void cmd_branch(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    if (argc == 0 || argv[0] == NULL) {
        list_branches();
        return;
    }

    const char *subcommand = argv[0];
    const char *target_name = (argc > 1) ? argv[1] : NULL;

    if (strcmp(subcommand, "merge") == 0) {
        if (!target_name) {
            printf("❌ Missing branch name to merge. Usage: lit branch merge <branch_name>\n");
            return;
        }
        merge_branch(target_name);
    } 
    else if (strcmp(subcommand, "-d") == 0 || strcmp(subcommand, "--delete") == 0) {
        if (!target_name) {
            printf("❌ Missing branch name to delete.\n");
            return;
        }
        char path[MAX_PATH];
        snprintf(path, sizeof(path), ".lit/refs/heads/%s", target_name);
        if (unlink(path) == 0) {
            printf("🗑️ Deleted branch '%s'\n", target_name);
        } else {
            printf("❌ Could not delete branch '%s'. It may not exist.\n", target_name);
        }
    }
    else {
        // Treat argument as either creating a new branch or switching to it
        // Check if branch exists, if not create it; or provide simple checkout flow
        struct stat st;
        char path[MAX_PATH];
        snprintf(path, sizeof(path), ".lit/refs/heads/%s", subcommand);

        if (stat(path, &st) == 0) {
            switch_branch(subcommand);
        } else {
            create_branch(subcommand);
            switch_branch(subcommand);
        }
    }
}