#include <stdio.h>
#include <stdlib.h>

// Forward declaration from engine.c
int engine_init(void);

void cmd_start(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (engine_init() == 0) {
        printf("🔥 Successfully initialized empty custom .lit repository!\n");
    } else {
        printf("⚠️ .lit workspace already exists or failed to initialize.\n");
    }
}