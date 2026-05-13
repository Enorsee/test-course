#include <iostream>

#include "analysis.h"
#include "cfg.h"

int main() {
    cfglab::CFG cfg;
    cfg.function_name = "manual_demo";
    cfg.entry = 1;
    cfg.exit = 6;
    cfg.nodes = {
        {1, "ENTRY"},
        {2, "x = x + 1"},
        {3, "if (x < 5)"},
        {4, "x = x + 2"},
        {5, "x = x + 1"},
        {6, "EXIT"},
    };
    cfg.edges = {
        {1, 2, ""},
        {2, 3, ""},
        {3, 4, "T"},
        {3, 5, "F"},
        {4, 3, "back"},
        {5, 6, ""},
    };

    const auto report = cfglab::compute_prime_path_coverage(cfg, 3);

    std::cout << "Prime paths for manual_demo:\n";
    for (const auto& path : report.prime_paths) {
        std::cout << "  " << cfglab::path_to_string(path) << '\n';
    }

    std::cout << "\nSelected test paths:\n";
    for (const auto& path : report.selected_test_paths) {
        std::cout << "  " << cfglab::path_to_string(path) << '\n';
    }

    return 0;
}
