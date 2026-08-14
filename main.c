#include <stdio.h>
#include <string.h>

// Forward declarations of command handlers
void cmd_start(int argc, char *argv[]);
void cmd_save(int argc, char *argv[]);
void cmd_status(int argc, char *argv[]);
void cmd_history(int argc, char *argv[]);
void cmd_ignite(int argc, char *argv[]);
void cmd_push(int argc, char *argv[]);
void cmd_branch(int argc, char *argv[]);
void cmd_checkout(int argc, char *argv[]);
void cmd_dilute(int argc, char *argv[]);
void cmd_dalek(int argc, char *argv[]);
void cmd_nuke(int argc, char *argv[]);
void cmd_help(int argc, char *argv[]);
int parse_flags(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    // If no arguments or help requested, print the logo and command manual
    if (argc < 2 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        cmd_help(0, NULL);
        return 0;
    }

    // Check if the user passed modifier flags (e.g., --remote or --ignore)
    if (parse_flags(argc - 1, &argv[1]) > 0) {
        return 0; // Flags successfully handled and processed
    }

    // Route command arguments to their respective implementation modules
    const char *command = argv[1];
    int sub_argc = argc - 2;
    char **sub_argv = &argv[2];

    if (strcmp(command, "start") == 0) {
        cmd_start(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "save") == 0) {
        cmd_save(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "status") == 0) {
        cmd_status(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "history") == 0) {
        cmd_history(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "ignite") == 0) {
        cmd_ignite(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "push") == 0) {
        cmd_push(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "branch") == 0) {
        cmd_branch(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "checkout") == 0) {
        cmd_checkout(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "dilute") == 0) {
        cmd_dilute(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "dalek") == 0) {
        cmd_dalek(sub_argc, sub_argv);
    } 
    else if (strcmp(command, "nuke") == 0) {
        cmd_nuke(sub_argc, sub_argv);
    } 
    else {
        printf("❌ Unknown command: '%s'\n", command);
        printf("💡 Run `lit help` to view all available commands and options.\n");
        return 1;
    }

    return 0;
}