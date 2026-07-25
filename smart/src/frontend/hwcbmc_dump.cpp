// hwcbmc_dump: the hw-cbmc-backed twin of frontend_dump.
//   hwcbmc_dump <file.sv> [top]     -> canonical JSON on stdout
//   hwcbmc_dump --strip <file.sv>   -> assume-stripped source on stdout
// Same CLI as frontend_dump so tools/parity_frontend.py can drive either.
// WP4 folds this into the `smart` binary as --dump-frontend.

#include <iostream>
#include <string>

#include "HwcbmcFrontend.h"
#include "SVModule.h"

#include <fstream>
#include <sstream>

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
            // Assume stripping stays textual — see plan WP2: a real AST would
            // force lossy pretty-printing of the user's source.
            std::cout << smart::frontend::stripAssumes(readFile(argv[2]));
            return 0;
        }
        if (argc < 2) {
            std::cerr << "Usage: hwcbmc_dump <file.sv> [top]\n"
                      << "       hwcbmc_dump --strip <file.sv>\n";
            return 1;
        }
        std::string file = argv[1];
        std::string top = argc >= 3 ? argv[2] : stem(file);

        smart::frontend::HwcbmcOptions options;
        auto info = smart::frontend::parseModuleFile(file, top, options);
        if (!options.elaborated) {
            std::cerr << "hwcbmc_dump: widths unresolved, elaboration failed: "
                      << options.elaborationError << "\n";
        }
        std::cout << smart::frontend::dumpJson(info) << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "hwcbmc_dump: " << e.what() << "\n";
        return 1;
    }
}
