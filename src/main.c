
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
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
    const char *build_date = KSH_BUILD_DATE;
    const char *target = KSH_TARGET;

    // If configure didn't substitute KSH_BUILD_DATE, fallback to compile date
    if (!build_date || strchr(build_date, '$') != NULL || strcmp(build_date, "unknown") == 0) {
        // __DATE__ has format "Mmm dd yyyy" (e.g. "Oct  2 2025"). Convert to YYYY-MM-DD
        const char *d = __DATE__;
        char month[4];
        int day, year;
        if (sscanf(d, "%3s %d %d", month, &day, &year) == 3) {
            static const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
            char monnum[3] = "00";
            const char *p = strstr(months, month);
            if (p) {
                int idx = (p - months) / 3 + 1;
                snprintf(monnum, sizeof(monnum), "%02d", idx);
            }
            static char buf[16];
            snprintf(buf, sizeof(buf), "%04d-%s-%02d", year, monnum, day);
            build_date = buf;
        } else {
            // last-resort: use current local date
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            static char buf2[16];
            strftime(buf2, sizeof(buf2), "%Y-%m-%d", &tm);
            build_date = buf2;
        }
    }

    if (!target || strchr(target, '$') != NULL || strcmp(target, "unknown") == 0) {
        struct utsname uts;
        if (uname(&uts) == 0) {
            static char tbuf[128];
            snprintf(tbuf, sizeof(tbuf), "%s-%s", uts.machine, uts.sysname);
            target = tbuf;
        } else {
            target = "unknown";
        }
    }

    printf("Build date: %s\n", build_date);
    printf("Target: %s\n", target);
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

     /* Export version info for tools like fastfetch/neofetch/screenfetch.
         These tools can read KSH_VERSION (and related vars) if present. */
     setenv("KSH_VERSION", KSH_VERSION, 1);
     setenv("KSH_BUILD_DATE", KSH_BUILD_DATE, 1);
     setenv("KSH_TARGET", KSH_TARGET, 1);

     /* Do not print a banner on interactive startup. Users generally prefer
         a quiet prompt; use --version to show version details. */
     shell_start(KSH_VERSION);
    // TODO: Shell main loop
    return 0;
}
