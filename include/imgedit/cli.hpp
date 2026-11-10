#pragma once

#include <string>
#include <vector>

#include "imgedit/image.hpp"

namespace imgedit {

// Applies one operation written as "name" or "name=value", e.g. "blur=2.5",
// "resize=800x600", "crop=10,10,200,100". Throws ImageError if it's invalid.
Image apply_operation(const Image& img, const std::string& op);

// Entry point of the command line tool. Returns the process exit code.
int run_cli(const std::vector<std::string>& args);

}  // namespace imgedit
