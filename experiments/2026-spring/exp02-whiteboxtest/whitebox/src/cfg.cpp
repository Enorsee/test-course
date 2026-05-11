#include "cfg.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "util.h"

namespace cfglab {

namespace {

struct PendingEdge {
    int from{0};
    std::string label;
};

struct BuildResult {
    int entry{-1};
    std::vector<PendingEdge> normal;
    std::vector<PendingEdge> breaks;
    std::vector<PendingEdge> continues;
};

class CFGBuilderImpl {
  public:
    explicit CFGBuilderImpl(const std::string& function_name)
        : function_name_(function_name) {}

    CFG build(const Function& function) {
        cfg_.function_name = function.name;
        const int entry = add_node("ENTRY");
        const int exit = add_node("EXIT");
        cfg_.entry = entry;
        cfg_.exit = exit;

        BuildResult body = build_statement(function.body);
        if (body.entry != -1) {
            add_edge(entry, body.entry, "");
            connect(body.normal, exit);
        } else {
            add_edge(entry, exit, "");
        }

        if (!body.breaks.empty() || !body.continues.empty()) {
            throw std::runtime_error("Found break/continue outside of a loop");
        }

        return cfg_;
    }

  private:
    int add_node(const std::string& label) {
        const int id = next_id_++;
        cfg_.nodes.push_back(CFGNode{id, label});
        return id;
    }

    void add_edge(int from, int to, const std::string& label) {
        cfg_.edges.push_back(CFGEdge{from, to, label});
    }

    void connect(const std::vector<PendingEdge>& pending_edges, int target) {
        for (const PendingEdge& edge : pending_edges) {
            add_edge(edge.from, target, edge.label);
        }
    }

    int build_action_node(const std::string& label) {
        return add_node(label.empty() ? "<empty>" : label);
    }

    BuildResult build_block(const std::shared_ptr<Stmt>& stmt) {
        BuildResult result;
        bool has_entry = false;

        for (const auto& child : stmt->children) {
            BuildResult current = build_statement(child);
            if (current.entry == -1) {
                continue;
            }
            if (!has_entry) {
                result.entry = current.entry;
                has_entry = true;
            } else {
                connect(result.normal, current.entry);
            }
            result.normal = current.normal;
            result.breaks.insert(result.breaks.end(), current.breaks.begin(),
                                 current.breaks.end());
            result.continues.insert(result.continues.end(), current.continues.begin(),
                                    current.continues.end());
        }

        if (!has_entry) {
            const int empty = build_action_node("<empty block>");
            result.entry = empty;
            result.normal = {{empty, ""}};
        }

        return result;
    }

    BuildResult build_if(const std::shared_ptr<Stmt>& stmt) {
        BuildResult result;
        const int condition = add_node("if (" + stmt->condition + ")");
        result.entry = condition;

        BuildResult then_branch = build_statement(stmt->then_branch);
        add_edge(condition, then_branch.entry, "T");

        result.normal.insert(result.normal.end(), then_branch.normal.begin(),
                             then_branch.normal.end());
        result.breaks.insert(result.breaks.end(), then_branch.breaks.begin(),
                             then_branch.breaks.end());
        result.continues.insert(result.continues.end(), then_branch.continues.begin(),
                                then_branch.continues.end());

        if (stmt->else_branch) {
            BuildResult else_branch = build_statement(stmt->else_branch);
            add_edge(condition, else_branch.entry, "F");
            result.normal.insert(result.normal.end(), else_branch.normal.begin(),
                                 else_branch.normal.end());
            result.breaks.insert(result.breaks.end(), else_branch.breaks.begin(),
                                 else_branch.breaks.end());
            result.continues.insert(result.continues.end(), else_branch.continues.begin(),
                                    else_branch.continues.end());
        } else {
            result.normal.push_back({condition, "F"});
        }

        return result;
    }

    BuildResult build_while(const std::shared_ptr<Stmt>& stmt) {
        BuildResult result;
        const int condition = add_node("while (" + stmt->condition + ")");
        result.entry = condition;

        BuildResult body = build_statement(stmt->body);
        add_edge(condition, body.entry, "T");
        connect(body.normal, condition);
        connect(body.continues, condition);

        result.normal.push_back({condition, "F"});
        result.breaks = body.breaks;
        return result;
    }

    BuildResult build_do_while(const std::shared_ptr<Stmt>& stmt) {
        BuildResult result;
        BuildResult body = build_statement(stmt->body);
        const int condition = add_node("do-while (" + stmt->condition + ")");

        result.entry = body.entry;
        connect(body.normal, condition);
        connect(body.continues, condition);
        add_edge(condition, body.entry, "T");
        result.normal.push_back({condition, "F"});
        result.breaks = body.breaks;
        return result;
    }

    BuildResult build_for(const std::shared_ptr<Stmt>& stmt) {
        BuildResult result;

        int init_node = -1;
        if (!trim(stmt->init).empty()) {
            init_node = add_node("for-init: " + stmt->init);
        }
        const std::string condition_text =
            trim(stmt->condition).empty() ? "true" : stmt->condition;
        const int condition = add_node("for-cond: " + condition_text);
        int update_node = -1;
        if (!trim(stmt->update).empty()) {
            update_node = add_node("for-update: " + stmt->update);
        }

        BuildResult body = build_statement(stmt->body);
        add_edge(condition, body.entry, "T");

        if (update_node != -1) {
            connect(body.normal, update_node);
            connect(body.continues, update_node);
            add_edge(update_node, condition, "back");
        } else {
            connect(body.normal, condition);
            connect(body.continues, condition);
        }

        if (init_node != -1) {
            add_edge(init_node, condition, "");
            result.entry = init_node;
        } else {
            result.entry = condition;
        }

        result.normal.push_back({condition, "F"});
        result.breaks = body.breaks;
        return result;
    }

    BuildResult build_statement(const std::shared_ptr<Stmt>& stmt) {
        if (!stmt) {
            const int empty = build_action_node("<null>");
            return BuildResult{empty, {{empty, ""}}, {}, {}};
        }

        switch (stmt->kind) {
            case StmtKind::Block:
                return build_block(stmt);
            case StmtKind::Expr:
            case StmtKind::Decl:
            case StmtKind::Empty: {
                const int node = build_action_node(stmt->text);
                return BuildResult{node, {{node, ""}}, {}, {}};
            }
            case StmtKind::Return: {
                const int node = build_action_node(stmt->text);
                add_edge(node, cfg_.exit, "return");
                return BuildResult{node, {}, {}, {}};
            }
            case StmtKind::Break: {
                const int node = build_action_node("break");
                return BuildResult{node, {}, {{node, "break"}}, {}};
            }
            case StmtKind::Continue: {
                const int node = build_action_node("continue");
                return BuildResult{node, {}, {}, {{node, "continue"}}};
            }
            case StmtKind::If:
                return build_if(stmt);
            case StmtKind::While:
                return build_while(stmt);
            case StmtKind::DoWhile:
                return build_do_while(stmt);
            case StmtKind::For:
                return build_for(stmt);
        }

        throw std::runtime_error("Unknown statement kind");
    }

    CFG cfg_;
    std::string function_name_;
    int next_id_{1};
};

}  // namespace

CFG CFGBuilder::build(const Function& function) const {
    return CFGBuilderImpl(function.name).build(function);
}

std::vector<std::vector<int>> CFG::adjacency_list() const {
    int max_id = 0;
    for (const CFGNode& node : nodes) {
        max_id = std::max(max_id, node.id);
    }
    std::vector<std::vector<int>> adjacency(static_cast<std::size_t>(max_id + 1));
    for (const CFGEdge& edge : edges) {
        adjacency[static_cast<std::size_t>(edge.from)].push_back(edge.to);
    }
    return adjacency;
}

std::string cfg_to_text(const CFG& cfg) {
    std::ostringstream stream;
    stream << "Function: " << cfg.function_name << '\n';
    stream << "Entry Node: " << cfg.entry << '\n';
    stream << "Exit Node: " << cfg.exit << '\n';
    stream << "Nodes:\n";
    for (const CFGNode& node : cfg.nodes) {
        stream << "  " << node.id << ": " << node.label << '\n';
    }
    stream << "Edges:\n";
    for (const CFGEdge& edge : cfg.edges) {
        stream << "  " << edge.from << " -> " << edge.to;
        if (!edge.label.empty()) {
            stream << " [" << edge.label << "]";
        }
        stream << '\n';
    }
    return stream.str();
}

std::string cfg_to_dot(const CFG& cfg) {
    std::ostringstream stream;
    stream << "digraph \"" << escape_dot_label(cfg.function_name) << "\" {\n";
    stream << "  rankdir=TB;\n";
    stream << "  node [shape=box, style=rounded];\n";
    for (const CFGNode& node : cfg.nodes) {
        std::string shape = "box";
        if (node.id == cfg.entry) {
            shape = "oval";
        } else if (node.id == cfg.exit) {
            shape = "doublecircle";
        }
        stream << "  " << node.id << " [label=\"" << escape_dot_label(node.label)
               << "\", shape=" << shape << "];\n";
    }
    for (const CFGEdge& edge : cfg.edges) {
        stream << "  " << edge.from << " -> " << edge.to;
        if (!edge.label.empty()) {
            stream << " [label=\"" << escape_dot_label(edge.label) << "\"]";
        }
        stream << ";\n";
    }
    stream << "}\n";
    return stream.str();
}

}  // namespace cfglab
