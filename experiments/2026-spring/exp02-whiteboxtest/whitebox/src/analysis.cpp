#include "analysis.h"

#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace cfglab {

std::vector<Path> select_minimal_cover(const std::vector<Path>& candidates,
                                       const std::vector<Path>& requirements) {
    if (requirements.empty()) {
        return {};
    }

    std::vector<Path> selected;
    
    // ********************************
    // ***   Write your code here   ***
    // ********************************
    
    return selected;
}

std::vector<Path> enumerate_prime_paths(const CFG& cfg) {
    std::vector<Path> prime_paths;
    
    // ********************************
    // ***   Write your code here   ***
    // ********************************

    return prime_paths;
}

std::vector<Path> enumerate_test_paths(const CFG& cfg, int max_visits) {
    if (max_visits < 1) {
        throw std::runtime_error("max_visits must be at least 1");
    }

    std::vector<Path> test_paths;
    
    // ********************************
    // ***   Write your code here   ***
    // ********************************
    
    return test_paths;
}

CoverageReport compute_prime_path_coverage(const CFG& cfg, int max_visits) {
    CoverageReport report;
    report.max_visits = max_visits;
    report.prime_paths = enumerate_prime_paths(cfg);
    report.candidate_test_paths = enumerate_test_paths(cfg, max_visits);
    report.selected_test_paths =
        select_minimal_cover(report.candidate_test_paths, report.prime_paths);
    return report;
}

std::string path_to_string(const Path& path) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < path.size(); ++index) {
        if (index > 0) {
            stream << '-';
        }
        stream << path[index];
    }
    return stream.str();
}

}  // namespace cfglab
