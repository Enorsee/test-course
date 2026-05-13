#pragma once

#include <string>
#include <vector>

namespace cfglab {

std::string trim(const std::string& value);
std::string join_strings(const std::vector<std::string>& values,
                         const std::string& separator = " ");
std::string escape_dot_label(const std::string& value);

}  // namespace cfglab
