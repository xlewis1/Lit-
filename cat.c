#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH 1024

int engine_is_initialized(void);

void cmd_cat(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    if (argc == 0 || argv[0] == NULL) {
        printf("❌ Missing object hash. Usage: lit cat <object_hash>\n");
        return;
    }

    const char *target = argv[0];
    size_t target_len = strlen(target);
    char full_path[MAX_PATH] = {0};
    char resolved_hash[65] = {0};

    DIR *dir = opendir(".lit/objects");
    if (!dir) {
        printf("❌ Could not open object storage directory.\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Check if the filename starts with the target prefix
        if (strncmp(entry->d_name, target, target_len) == 0) {
            snprintf(full_path, sizeof(full_path), ".lit/objects/%s", entry->d_name);
            snprintf(resolved_hash, sizeof(resolved_hash), "%s", entry->d_name);
            break;
        }
    }
    closedir(dir);

    if (strlen(full_path) == 0) {
        printf("❌ Object matching prefix '%s' not found in .lit/objects/.\n", target);
        return;
    }

    FILE *f = fopen(full_path, "r");
    if (!f) {
        printf("❌ Unable to read object file.\n");
        return;
    }

    printf("📄 Object Content [%.12s]:\n", resolved_hash);
    printf("----------------------------------------\n");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), f)) {
        printf("%s", buffer);
    }
    printf("----------------------------------------\n");

    fclose(f);
}