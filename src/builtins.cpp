
#include <iostream>
#include "../include/ksh.h"

void ksh_help() {
    std::cout << "Kuznix Shell Help: Built-in commands..." << std::endl;
    std::cout << "  help      Show help" << std::endl;
    std::cout << "  version   Show version info" << std::endl;
}

void ksh_version() {
#ifdef KSH_VERSION
    std::cout << "ksh (Kuznix Shell) version " << KSH_VERSION << std::endl;
#else
    std::cout << "ksh (Kuznix Shell) version unknown" << std::endl;
#endif
#ifdef KSH_BUILD_DATE
    std::cout << "Build date: " << KSH_BUILD_DATE << std::endl;
#endif
#ifdef KSH_TARGET
    std::cout << "Target: " << KSH_TARGET << std::endl;
#endif
}
