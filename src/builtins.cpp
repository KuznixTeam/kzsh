
#include <iostream>
#include "../include/ksh.h"

extern "C" void print_version();

void ksh_help() {
    std::cout << "Kuznix Shell Help: Built-in commands..." << std::endl;
    std::cout << "  help      Show help" << std::endl;
    std::cout << "  version   Show version info" << std::endl;
}

void ksh_version() {
    /* Reuse the C print_version() routine so both the binary --version and
       the built-in version command produce identical output. */
    print_version();
}
