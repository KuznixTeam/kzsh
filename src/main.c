
#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "utils.h"
#ifndef KSH_VERSION
#define KSH_VERSION "0.1"
#endif
#ifndef KSH_BUILD_DATE
#define KSH_BUILD_DATE "unknown"
#endif
#ifndef KSH_TARGET
#define KSH_TARGET "unknown"
#endif

void print_version() {
    printf("ksh (Kuznix Shell) version %s\n", KSH_VERSION);
    printf("Build date: %s\n", KSH_BUILD_DATE);
    printf("Target: %s\n", KSH_TARGET);
}

void print_help() {
    printf("Usage: ksh [options]\n");
    printf("Options:\n");
    printf("  --version     Show version info\n");
    printf("  --help        Show this help message\n");
    printf("  -c <command>  Execute command\n");
}


int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[1], "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[1], "-c") == 0 && argc > 2) {
            printf("Executing: %s\n", argv[2]);
            // TODO: Execute command
            return 0;
        }
    }

    print_banner(KSH_VERSION);
    shell_start(KSH_VERSION);
    // TODO: Shell main loop
    return 0;
}
