#include "parser.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <unordered_set>

#include "util.h"

namespace cfglab {

namespace {

const std::unordered_set<std::string> kDeclarationStarters = {
    "int",      "char",   "float",   "double", "long",   "short",
    "void",     "unsigned", "signed", "const",  "static", "bool",
    "struct",   "enum"
};

std::shared_ptr<Stmt> make_stmt(StmtKind kind) {
    auto stmt = std::make_shared<Stmt>();
    stmt->kind = kind;
    return stmt;
}

}  // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

Program Parser::parse_program() {
    Program program;

    while (!is_at_end()) {
        std::vector<Token> header_tokens;
        int paren_depth = 0;
        bool saw_open_brace = false;

        while (!is_at_end()) {
            if (check("(")) {
                ++paren_depth;
            } else if (check(")")) {
                --paren_depth;
            }

            if (paren_depth == 0 && check(";")) {
                advance();
                header_tokens.clear();
                break;
            }
            if (paren_depth == 0 && check("{")) {
                saw_open_brace = true;
                break;
            }

            header_tokens.push_back(advance());
        }

        if (header_tokens.empty()) {
            continue;
        }
        if (!saw_open_brace) {
            fail_here("Expected a function body after declaration header");
        }

        expect("{", "Expected '{' to start a function body");
        program.functions.push_back(parse_function(header_tokens));
    }

    return program;
}

const Token& Parser::peek(int offset) const {
    const std::size_t index = position_ + static_cast<std::size_t>(offset);
    if (index >= tokens_.size()) {
        return tokens_.back();
    }
    return tokens_[index];
}

const Token& Parser::advance() {
    if (!is_at_end()) {
        ++position_;
    }
    return tokens_[position_ - 1];
}

bool Parser::is_at_end() const {
    return peek().kind == TokenKind::End;
}

bool Parser::check(const std::string& text) const {
    return peek().text == text;
}

bool Parser::check_keyword(const std::string& text) const {
    return peek().kind == TokenKind::Keyword && peek().text == text;
}

bool Parser::match(const std::string& text) {
    if (!check(text)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::match_keyword(const std::string& text) {
    if (!check_keyword(text)) {
        return false;
    }
    advance();
    return true;
}

void Parser::expect(const std::string& text, const std::string& message) {
    if (!match(text)) {
        fail_here(message);
    }
}

[[noreturn]] void Parser::fail_here(const std::string& message) const {
    std::ostringstream stream;
    stream << message << " near token '" << peek().text << "' at line " << peek().line
           << ", column " << peek().column;
    throw ParseError(stream.str());
}

Function Parser::parse_function(const std::vector<Token>& header_tokens) {
    const auto open_paren = std::find_if(header_tokens.begin(), header_tokens.end(),
                                         [](const Token& token) { return token.text == "("; });
    const auto close_paren = std::find_if(header_tokens.rbegin(), header_tokens.rend(),
                                          [](const Token& token) { return token.text == ")"; });
    if (open_paren == header_tokens.end() || close_paren == header_tokens.rend()) {
        throw ParseError("Malformed function header");
    }

    const std::size_t open_index =
        static_cast<std::size_t>(std::distance(header_tokens.begin(), open_paren));
    const std::size_t close_index = static_cast<std::size_t>(
        header_tokens.size() - 1 - std::distance(header_tokens.rbegin(), close_paren));
    if (open_index == 0 || close_index <= open_index) {
        throw ParseError("Malformed function header");
    }

    const Token& name_token = header_tokens[open_index - 1];
    if (!is_identifier_like(name_token)) {
        throw ParseError("Could not determine function name");
    }

    std::vector<std::string> return_type_tokens;
    for (std::size_t index = 0; index + 1 < open_index; ++index) {
        return_type_tokens.push_back(header_tokens[index].text);
    }

    std::vector<std::string> parameter_tokens;
    for (std::size_t index = open_index + 1; index < close_index; ++index) {
        parameter_tokens.push_back(header_tokens[index].text);
    }

    Function function;
    function.return_type = trim(join_strings(return_type_tokens));
    function.name = name_token.text;
    function.parameters = trim(join_strings(parameter_tokens));
    function.body = parse_compound_statement();
    return function;
}

std::shared_ptr<Stmt> Parser::parse_compound_statement() {
    auto block = make_stmt(StmtKind::Block);
    while (!check("}") && !is_at_end()) {
        block->children.push_back(parse_statement());
    }
    expect("}", "Expected '}' to end a compound statement");
    return block;
}

std::shared_ptr<Stmt> Parser::parse_statement() {
    if (match("{")) {
        return parse_compound_statement();
    }
    if (match_keyword("if")) {
        return parse_if_statement();
    }
    if (match_keyword("while")) {
        return parse_while_statement();
    }
    if (match_keyword("do")) {
        return parse_do_while_statement();
    }
    if (match_keyword("for")) {
        return parse_for_statement();
    }
    if (match_keyword("return")) {
        return parse_return_statement();
    }
    if (match_keyword("break")) {
        return parse_break_statement();
    }
    if (match_keyword("continue")) {
        return parse_continue_statement();
    }
    if (match(";")) {
        auto stmt = make_stmt(StmtKind::Empty);
        stmt->text = "<empty>";
        return stmt;
    }
    return parse_simple_statement();
}

std::shared_ptr<Stmt> Parser::parse_if_statement() {
    auto stmt = make_stmt(StmtKind::If);
    stmt->condition = tokens_to_text(collect_parenthesized_tokens());
    stmt->then_branch = parse_statement();
    if (match_keyword("else")) {
        stmt->else_branch = parse_statement();
    }
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_while_statement() {
    auto stmt = make_stmt(StmtKind::While);
    stmt->condition = tokens_to_text(collect_parenthesized_tokens());
    stmt->body = parse_statement();
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_do_while_statement() {
    auto stmt = make_stmt(StmtKind::DoWhile);
    stmt->body = parse_statement();
    if (!match_keyword("while")) {
        fail_here("Expected 'while' after 'do' body");
    }
    stmt->condition = tokens_to_text(collect_parenthesized_tokens());
    expect(";", "Expected ';' after do-while");
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_for_statement() {
    auto stmt = make_stmt(StmtKind::For);
    const std::vector<Token> parts = collect_parenthesized_tokens();
    const auto split = split_top_level(parts, ";");
    if (split.size() != 3) {
        throw ParseError("Expected three components inside a for loop header");
    }
    stmt->init = tokens_to_text(split[0]);
    stmt->condition = tokens_to_text(split[1]);
    stmt->update = tokens_to_text(split[2]);
    stmt->body = parse_statement();
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_return_statement() {
    auto stmt = make_stmt(StmtKind::Return);
    const std::vector<Token> tokens = collect_until_top_level_semicolon();
    stmt->text = tokens.empty() ? "return" : "return " + tokens_to_text(tokens);
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_break_statement() {
    expect(";", "Expected ';' after break");
    auto stmt = make_stmt(StmtKind::Break);
    stmt->text = "break";
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_continue_statement() {
    expect(";", "Expected ';' after continue");
    auto stmt = make_stmt(StmtKind::Continue);
    stmt->text = "continue";
    return stmt;
}

std::shared_ptr<Stmt> Parser::parse_simple_statement() {
    const std::vector<Token> tokens = collect_until_top_level_semicolon();
    if (tokens.empty()) {
        auto stmt = make_stmt(StmtKind::Empty);
        stmt->text = "<empty>";
        return stmt;
    }

    auto stmt = make_stmt(looks_like_declaration(tokens) ? StmtKind::Decl : StmtKind::Expr);
    stmt->text = tokens_to_text(tokens);
    return stmt;
}

std::vector<Token> Parser::collect_parenthesized_tokens() {
    expect("(", "Expected '('");
    std::vector<Token> collected;
    int depth = 1;
    while (!is_at_end() && depth > 0) {
        const Token token = advance();
        if (token.text == "(") {
            ++depth;
        } else if (token.text == ")") {
            --depth;
            if (depth == 0) {
                break;
            }
        }
        collected.push_back(token);
    }
    if (depth != 0) {
        throw ParseError("Unbalanced parentheses in expression");
    }
    return collected;
}

std::vector<Token> Parser::collect_until_top_level_semicolon() {
    std::vector<Token> collected;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;

    while (!is_at_end()) {
        if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0 && check(";")) {
            advance();
            return collected;
        }

        const Token token = advance();
        if (token.text == "(") {
            ++paren_depth;
        } else if (token.text == ")") {
            --paren_depth;
        } else if (token.text == "[") {
            ++bracket_depth;
        } else if (token.text == "]") {
            --bracket_depth;
        } else if (token.text == "{") {
            ++brace_depth;
        } else if (token.text == "}") {
            --brace_depth;
        }
        collected.push_back(token);
    }

    throw ParseError("Missing ';' at end of statement");
}

std::vector<std::vector<Token>> Parser::split_top_level(const std::vector<Token>& tokens,
                                                        const std::string& separator) const {
    std::vector<std::vector<Token>> parts(1);
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;

    for (const Token& token : tokens) {
        if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0 &&
            token.text == separator) {
            parts.emplace_back();
            continue;
        }
        if (token.text == "(") {
            ++paren_depth;
        } else if (token.text == ")") {
            --paren_depth;
        } else if (token.text == "[") {
            ++bracket_depth;
        } else if (token.text == "]") {
            --bracket_depth;
        } else if (token.text == "{") {
            ++brace_depth;
        } else if (token.text == "}") {
            --brace_depth;
        }
        parts.back().push_back(token);
    }

    return parts;
}

std::string Parser::tokens_to_text(const std::vector<Token>& tokens) const {
    std::vector<std::string> texts;
    texts.reserve(tokens.size());
    for (const Token& token : tokens) {
        texts.push_back(token.text);
    }
    return trim(join_strings(texts));
}

bool Parser::looks_like_declaration(const std::vector<Token>& tokens) const {
    if (tokens.empty()) {
        return false;
    }
    return kDeclarationStarters.count(tokens.front().text) > 0;
}

}  // namespace cfglab
