#include "../include/utils.h"
#include <iostream>
#include <fstream>
#include <string>

void print_banner(const std::string& version) {
    std::cout << "kzsh-" << version << std::endl;
}

bool load_kshrc() {
    const char* home = getenv("HOME");
    if (!home) return false;
    std::string rcfile = std::string(home) + "/.kshrc";
    std::ifstream rc(rcfile);
    if (rc) {
        std::cout << "Loading .kshrc..." << std::endl;
        // TODO: parse and execute .kshrc
        rc.close();
        return true;
    }
    return false;
}
