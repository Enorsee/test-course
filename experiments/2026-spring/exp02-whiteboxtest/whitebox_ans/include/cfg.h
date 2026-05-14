#pragma once

#include <string>
#include <vector>

#include "ast.h"

namespace cfglab {

struct CFGNode {
    int id{0};
    std::string label;
};

struct CFGEdge {
    int from{0};
    int to{0};
    std::string label;
};

struct CFG {
    std::string function_name;
    int entry{-1};
    int exit{-1};
    std::vector<CFGNode> nodes;
    std::vector<CFGEdge> edges;

    std::vector<std::vector<int>> adjacency_list() const;
};

class CFGBuilder {
  public:
    CFG build(const Function& function) const;
};

std::string cfg_to_text(const CFG& cfg);
std::string cfg_to_dot(const CFG& cfg);

}  // namespace cfglab
