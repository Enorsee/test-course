#include "analysis.h"

#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace cfglab {

namespace {

bool is_subpath(const Path& needle, const Path& haystack) {
    if (needle.size() > haystack.size()) {
        return false;
    }
    for (std::size_t start = 0; start + needle.size() <= haystack.size(); ++start) {
        bool matches = true;
        for (std::size_t index = 0; index < needle.size(); ++index) {
            if (haystack[start + index] != needle[index]) {
                matches = false;
                break;
            }
        }
        if (matches) {
            return true;
        }
    }
    return false;
}

bool path_covers_requirement(const Path& test_path, const Path& requirement) {
    return is_subpath(requirement, test_path);
}

void enumerate_simple_paths_from(const std::vector<std::vector<int>>& adjacency, int start,
                                 int current, std::vector<int>& path,
                                 std::unordered_map<int, int>& visit_count,
                                 std::set<Path>& simple_paths) {
    simple_paths.insert(path);

    for (int next : adjacency[static_cast<std::size_t>(current)]) {
        if (next == start && path.size() > 1) {
            Path cycle = path;
            cycle.push_back(start);
            simple_paths.insert(cycle);
            continue;
        }
        if (visit_count[next] > 0) {
            continue;
        }
        visit_count[next] = 1;
        path.push_back(next);
        enumerate_simple_paths_from(adjacency, start, next, path, visit_count, simple_paths);
        path.pop_back();
        visit_count.erase(next);
    }
}

std::vector<Path> select_minimal_cover(const std::vector<Path>& candidates,
                                       const std::vector<Path>& requirements) {
    if (requirements.empty()) {
        return {};
    }

    std::vector<std::vector<int>> candidate_covers(candidates.size());
    std::vector<std::vector<int>> requirement_coverers(requirements.size());

    for (std::size_t candidate_index = 0; candidate_index < candidates.size();
         ++candidate_index) {
        for (std::size_t requirement_index = 0; requirement_index < requirements.size();
             ++requirement_index) {
            if (path_covers_requirement(candidates[candidate_index],
                                        requirements[requirement_index])) {
                candidate_covers[candidate_index].push_back(
                    static_cast<int>(requirement_index));
                requirement_coverers[requirement_index].push_back(
                    static_cast<int>(candidate_index));
            }
        }
    }

    for (std::size_t requirement_index = 0; requirement_index < requirements.size();
         ++requirement_index) {
        if (requirement_coverers[requirement_index].empty()) {
            throw std::runtime_error(
                "Could not cover every prime path with the current max-visit bound");
        }
    }

    std::vector<int> coverage_count(requirements.size(), 0);
    int covered_requirements = 0;
    std::vector<int> chosen;
    std::vector<int> best_choice;
    int best_size = std::numeric_limits<int>::max();
    int best_total_length = std::numeric_limits<int>::max();

    const auto apply_candidate = [&](int candidate_index, std::vector<int>& changed) {
        changed.clear();
        for (int requirement_index : candidate_covers[candidate_index]) {
            if (coverage_count[static_cast<std::size_t>(requirement_index)] == 0) {
                ++covered_requirements;
                changed.push_back(requirement_index);
            }
            ++coverage_count[static_cast<std::size_t>(requirement_index)];
        }
    };

    const auto undo_candidate = [&](const std::vector<int>& changed, int candidate_index) {
        for (int requirement_index : candidate_covers[candidate_index]) {
            --coverage_count[static_cast<std::size_t>(requirement_index)];
        }
        covered_requirements -= static_cast<int>(changed.size());
    };

    std::function<void()> search = [&]() {
        if (static_cast<int>(chosen.size()) >= best_size) {
            return;
        }
        if (covered_requirements == static_cast<int>(requirements.size())) {
            int total_length = 0;
            for (int candidate_index : chosen) {
                total_length += static_cast<int>(candidates[candidate_index].size());
            }
            if (static_cast<int>(chosen.size()) < best_size ||
                (static_cast<int>(chosen.size()) == best_size &&
                 total_length < best_total_length)) {
                best_size = static_cast<int>(chosen.size());
                best_total_length = total_length;
                best_choice = chosen;
            }
            return;
        }

        int target_requirement = -1;
        std::size_t fewest_coverers = std::numeric_limits<std::size_t>::max();
        for (std::size_t requirement_index = 0; requirement_index < requirements.size();
             ++requirement_index) {
            if (coverage_count[requirement_index] > 0) {
                continue;
            }
            const std::size_t coverers =
                requirement_coverers[requirement_index].size();
            if (coverers < fewest_coverers) {
                fewest_coverers = coverers;
                target_requirement = static_cast<int>(requirement_index);
            }
        }

        std::vector<int> options =
            requirement_coverers[static_cast<std::size_t>(target_requirement)];
        std::sort(options.begin(), options.end(), [&](int left, int right) {
            const std::size_t left_score = candidate_covers[static_cast<std::size_t>(left)].size();
            const std::size_t right_score =
                candidate_covers[static_cast<std::size_t>(right)].size();
            if (left_score != right_score) {
                return left_score > right_score;
            }
            return candidates[static_cast<std::size_t>(left)].size() <
                   candidates[static_cast<std::size_t>(right)].size();
        });

        for (int candidate_index : options) {
            std::vector<int> changed;
            apply_candidate(candidate_index, changed);
            if (!changed.empty()) {
                chosen.push_back(candidate_index);
                search();
                chosen.pop_back();
            }
            undo_candidate(changed, candidate_index);
        }
    };

    search();

    std::vector<Path> selected;
    for (int candidate_index : best_choice) {
        selected.push_back(candidates[static_cast<std::size_t>(candidate_index)]);
    }
    return selected;
}

}  // namespace

std::vector<Path> enumerate_prime_paths(const CFG& cfg) {
    std::set<Path> all_simple_paths;
    const auto adjacency = cfg.adjacency_list();

    for (const CFGNode& node : cfg.nodes) {
        Path path = {node.id};
        std::unordered_map<int, int> visit_count;
        visit_count[node.id] = 1;
        enumerate_simple_paths_from(adjacency, node.id, node.id, path, visit_count,
                                    all_simple_paths);
    }

    std::vector<Path> simple_paths(all_simple_paths.begin(), all_simple_paths.end());
    std::vector<Path> prime_paths;
    for (std::size_t i = 0; i < simple_paths.size(); ++i) {
        bool contained = false;
        for (std::size_t j = 0; j < simple_paths.size(); ++j) {
            if (i == j) {
                continue;
            }
            if (simple_paths[j].size() <= simple_paths[i].size()) {
                continue;
            }
            if (is_subpath(simple_paths[i], simple_paths[j])) {
                contained = true;
                break;
            }
        }
        if (!contained) {
            prime_paths.push_back(simple_paths[i]);
        }
    }

    std::sort(prime_paths.begin(), prime_paths.end(), [](const Path& left, const Path& right) {
        if (left.size() != right.size()) {
            return left.size() < right.size();
        }
        return left < right;
    });
    return prime_paths;
}

std::vector<Path> enumerate_bounded_complete_paths(const CFG& cfg, int max_visits) {
    if (max_visits < 1) {
        throw std::runtime_error("max_visits must be at least 1");
    }

    const auto adjacency = cfg.adjacency_list();
    std::set<Path> complete_paths;
    Path current = {cfg.entry};
    std::unordered_map<int, int> visit_count;
    visit_count[cfg.entry] = 1;

    std::function<void(int)> dfs = [&](int node) {
        if (node == cfg.exit) {
            complete_paths.insert(current);
            return;
        }

        for (int next : adjacency[static_cast<std::size_t>(node)]) {
            const int count = visit_count[next];
            if (count >= max_visits) {
                continue;
            }
            visit_count[next] = count + 1;
            current.push_back(next);
            dfs(next);
            current.pop_back();
            if (count == 0) {
                visit_count.erase(next);
            } else {
                visit_count[next] = count;
            }
        }
    };

    dfs(cfg.entry);
    return std::vector<Path>(complete_paths.begin(), complete_paths.end());
}

CoverageReport compute_prime_path_coverage(const CFG& cfg, int max_visits) {
    CoverageReport report;
    report.max_visits = max_visits;
    report.prime_paths = enumerate_prime_paths(cfg);
    report.candidate_test_paths = enumerate_bounded_complete_paths(cfg, max_visits);
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
