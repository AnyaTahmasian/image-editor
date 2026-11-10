#include <string>
#include <vector>

#include "imgedit/cli.hpp"

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    return imgedit::run_cli(args);
}
