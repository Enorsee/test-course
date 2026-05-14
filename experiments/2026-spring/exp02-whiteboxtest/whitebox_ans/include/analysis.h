#pragma once

#include <string>
#include <vector>

#include "cfg.h"

namespace cfglab {

using Path = std::vector<int>;

struct CoverageReport {
    std::vector<Path> prime_paths;
    std::vector<Path> candidate_test_paths;
    std::vector<Path> selected_test_paths;
    int max_visits{2};
};

std::vector<Path> enumerate_prime_paths(const CFG& cfg);
std::vector<Path> enumerate_bounded_complete_paths(const CFG& cfg, int max_visits);
CoverageReport compute_prime_path_coverage(const CFG& cfg, int max_visits);
std::string path_to_string(const Path& path);

}  // namespace cfglab
