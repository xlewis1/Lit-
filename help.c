#include <stdio.h>

// ANSI Escape Code for Bright Orange/Yellow-Orange styling
#define COLOR_ORANGE "\033[38;5;208m"
#define COLOR_CYAN   "\033[36m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_RESET  "\033[0m"

void cmd_help(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // Render the thick block ASCII art logo for LIT
    printf(COLOR_ORANGE);
    printf("  ██╗     ██╗████████╗\n");
    printf("  ██║     ██║╚══██╔══╝\n");
    printf("  ██║     ██║   ██║   \n");
    printf("  ██║     ██║   ██║   \n");
    printf("  ███████╗██║   ██║   \n");
    printf("  ╚══════╝╚═╝   ╚═╝   \n");
    printf("🔥 The High-Performance Native Git Tool\n");
    printf(COLOR_RESET);

    printf(COLOR_CYAN "--------------------------------------------------\n" COLOR_RESET);
    printf("Usage: lit <command> [arguments]\n\n");

    printf(COLOR_GREEN "Available Commands:\n" COLOR_RESET);
    printf("  %-12s - Initialize an empty .lit repository workspace\n", "start");
    printf("  %-12s - Show working tree status and untracked files\n", "status");
    printf("  %-12s - Save a snapshot of your files with a custom message\n", "save <msg>");
    printf("  %-12s - Display the commit history tree with timestamps\n", "history");
    printf("  %-12s - Pull and fast-forward updates from a remote/peer\n", "ignite");
    printf("  %-12s - Push your local commits and objects over network socket\n", "push <ip>");
    printf("  %-12s - Manage or switch active project branches\n", "branch");
    printf("  %-12s - Clean or dilute working directory cache\n", "dilute");
    printf("  %-12s - Nuke or delete repository objects/workspace\n", "nuke");
    printf("  %-12s - Print this help screen and logo\n", "help");
    printf(COLOR_CYAN "--------------------------------------------------\n" COLOR_RESET);
}