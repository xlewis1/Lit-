#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>

#define PORT 8192
#define MAX_PATH 1024
#define MAX_BUFFER 4096

// Forward declarations from engine.c
int engine_is_initialized(void);
int engine_get_head(char *buf, size_t max_len);

// Helper to send individual objects stored in .lit/objects over the socket
void push_objects_over_network(int sock) {
    DIR *dir = opendir(".lit/objects");
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char obj_path[MAX_PATH];
        snprintf(obj_path, sizeof(obj_path), ".lit/objects/%s", entry->d_name);

        FILE *f = fopen(obj_path, "rb");
        if (f) {
            // Send object header marker, filename (hash), and content length/data
            dprintf(sock, "OBJ %s\n", entry->d_name);
            
            char chunk[MAX_BUFFER];
            size_t bytes_read;
            while ((bytes_read = fread(chunk, 1, sizeof(chunk), f)) > 0) {
                send(sock, chunk, bytes_read, 0);
            }
            fclose(f);
            
            // Send object terminator
            dprintf(sock, "\n---END_OBJ---\n");
        }
    }
    closedir(dir);
}

void cmd_push(int argc, char *argv[]) {
    if (!engine_is_initialized()) {
        printf("❌ Not a .lit repository. Run `lit start` first.\n");
        return;
    }

    const char *target_host = "127.0.0.1"; // Default remote server host / IP
    if (argc > 0 && argv[0] != NULL) {
        target_host = argv[0];
    }

    printf("🔥 Connecting to remote lit target at %s:%d...\n", target_host, PORT);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("❌ Socket creation failed");
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, target_host, &serv_addr.sin_addr) <= 0) {
        printf("❌ Invalid or unsupported host address: %s\n", target_host);
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("❌ Connection failed. Is the remote server listener running?");
        close(sock);
        return;
    }

    // Get current HEAD reference to sync up stream
    char head_ref[256];
    if (engine_get_head(head_ref, sizeof(head_ref)) != 0) {
        printf("❌ Failed to read local HEAD pointer.\n");
        close(sock);
        return;
    }

    printf("🚀 Uploading reference and object database to remote...\n");
    
    // Send branch pointer info
    dprintf(sock, "REF %s\n", head_ref);

    // Stream all local version objects across the network socket
    push_objects_over_network(sock);

    // Send completion signal
    dprintf(sock, "DONE\n");

    printf("✨ Successfully pushed changes to remote host!\n");
    close(sock);
}