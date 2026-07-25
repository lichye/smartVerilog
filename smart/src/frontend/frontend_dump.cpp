// frontend_dump: debug/parity CLI around SVModule.
//   frontend_dump <file.sv> [top]          -> canonical JSON on stdout
//   frontend_dump --strip <file.sv>        -> assume-stripped source on stdout
// WP4 folds this into the main `smart` binary as --dump-frontend; until
// then it builds standalone (see build line in the progress doc).

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "SVModule.h"

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static std::string stem(const std::string& path) {
    size_t slash = path.find_last_of('/');
    std::string base = slash == std::string::npos ? path : path.substr(slash + 1);
    size_t dot = base.find_last_of('.');
    return dot == std::string::npos ? base : base.substr(0, dot);
}

int main(int argc, char** argv) {
    try {
        if (argc >= 3 && std::string(argv[1]) == "--strip") {
            std::cout << smart::frontend::stripAssumes(readFile(argv[2]));
            return 0;
        }
        if (argc < 2) {
            std::cerr << "Usage: frontend_dump <file.sv> [top]\n"
                      << "       frontend_dump --strip <file.sv>\n";
            return 1;
        }
        std::string file = argv[1];
        std::string top = argc >= 3 ? argv[2] : stem(file);
        auto info = smart::frontend::parseModule(readFile(file), top);
        std::cout << smart::frontend::dumpJson(info) << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "frontend_dump: " << e.what() << "\n";
        return 1;
    }
}
