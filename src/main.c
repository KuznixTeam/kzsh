#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

/* Build-time defines (provided by Meson) */
#ifndef KSH_RELEASE
#define KSH_RELEASE "unknown"
#endif
#ifndef KSH_BUILD_DATE
#define KSH_BUILD_DATE "1970-01-01"
#endif
#ifndef KSH_TARGET
#define KSH_TARGET "unknown-target"
#endif
#ifndef KSH_CURRENT_YEAR
#define KSH_CURRENT_YEAR "1970"
#endif

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* Line-buffer stdout for interactive responsiveness */
    setvbuf(stdout, NULL, _IOLBF, 0);

    /* Set KSH_VERSION environment variable for compatibility (shell_start will ensure)
     * and then start the shell frontend (banner and prompt handled there).
     */
    shell_start(KSH_RELEASE);

    return 0;
}
