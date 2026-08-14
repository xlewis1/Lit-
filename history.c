#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#define MAX_PATH 1024
#define MAX_CONTENT (1024 * 1024)

// ANSI Color Codes
#define COLOR_CYAN    "\033[36m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RESET   "\033[0m"

// Forward declaration from engine.c
int engine_is_initialized(void);

// Helper to read file contents
static char *read_file_to_str(const char *path, size_t *out_len) {
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

void cmd_history(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    DIR *dir = opendir(".lit/objects");
    if (!dir) {
        printf("❌ Failed to open object database.\n");
        return;
    }

    printf(COLOR_CYAN "🔥 Repository Commit History Tree:\n" COLOR_RESET);
    printf("----------------------------------------\n");

    struct dirent *entry;
    int commit_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (strncmp(entry->d_name, "temp_", 5) == 0) continue; // skip temporary staging files

        char path[MAX_PATH];
        snprintf(path, sizeof(path), ".lit/objects/%s", entry->d_name);

        size_t len = 0;
        char *content = read_file_to_str(path, &len);
        if (!content) continue;

        // Check if object starts with timestamp metadata indicating a commit object
        if (strstr(content, "timestamp ") != NULL && strstr(content, "message ") != NULL) {
            commit_count++;
            
            long timestamp = 0;
            char message[256] = "No message";

            // Parse timestamp and message from commit content manifest
            char *ts_ptr = strstr(content, "timestamp ");
            if (ts_ptr) {
                timestamp = atol(ts_ptr + 10);
            }

            char *msg_ptr = strstr(content, "message ");
            if (msg_ptr) {
                char *newline = strchr(msg_ptr, '\n');
                if (newline) *newline = '\0';
                strncpy(message, msg_ptr + 8, sizeof(message) - 1);
            }

            // Format timestamp into human-readable date string
            time_t t = (time_t)timestamp;
            struct tm *timeinfo = localtime(&t);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

            // Render tree node with cyan hash and yellow details
            printf(COLOR_CYAN "  * 📦 [%.12s]" COLOR_RESET " \n", entry->d_name);
            printf(COLOR_YELLOW "    ├── Time: %s\n", time_str);
            printf("    └── Msg : \"%s\"\n" COLOR_RESET, message);
            printf("----------------------------------------\n");
        }

        free(content);
    }
    closedir(dir);

    if (commit_count == 0) {
        printf("  🌿 No commit snapshots found yet. Run `lit save <message>` to create one.\n");
        printf("----------------------------------------\n");
    }
}