// src/version.c
#include <stdio.h>
#include "version.h"

void print_version(void) {
    printf("kzsh version %s\n", KSH_RELEASE);
}
