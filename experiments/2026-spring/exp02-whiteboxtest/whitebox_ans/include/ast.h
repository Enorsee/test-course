#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cfglab {

enum class StmtKind {
    Block,
    Expr,
    Decl,
    If,
    While,
    DoWhile,
    For,
    Return,
    Break,
    Continue,
    Empty
};

struct Stmt {
    StmtKind kind{StmtKind::Empty};
    std::string text;
    std::string condition;
    std::string init;
    std::string update;
    std::vector<std::shared_ptr<Stmt>> children;
    std::shared_ptr<Stmt> then_branch;
    std::shared_ptr<Stmt> else_branch;
    std::shared_ptr<Stmt> body;
};

struct Function {
    std::string return_type;
    std::string name;
    std::string parameters;
    std::shared_ptr<Stmt> body;
};

struct Program {
    std::vector<Function> functions;
};

}  // namespace cfglab
