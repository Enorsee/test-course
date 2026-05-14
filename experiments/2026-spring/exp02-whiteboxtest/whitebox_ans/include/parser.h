#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "ast.h"
#include "tokenizer.h"

namespace cfglab {

class ParseError : public std::runtime_error {
  public:
    explicit ParseError(const std::string& message)
        : std::runtime_error(message) {}
};

class Parser {
  public:
    explicit Parser(std::vector<Token> tokens);

    Program parse_program();

  private:
    const Token& peek(int offset = 0) const;
    const Token& advance();
    bool is_at_end() const;
    bool check(const std::string& text) const;
    bool check_keyword(const std::string& text) const;
    bool match(const std::string& text);
    bool match_keyword(const std::string& text);
    void expect(const std::string& text, const std::string& message);
    [[noreturn]] void fail_here(const std::string& message) const;

    Function parse_function(const std::vector<Token>& header_tokens);
    std::shared_ptr<Stmt> parse_compound_statement();
    std::shared_ptr<Stmt> parse_statement();
    std::shared_ptr<Stmt> parse_if_statement();
    std::shared_ptr<Stmt> parse_while_statement();
    std::shared_ptr<Stmt> parse_do_while_statement();
    std::shared_ptr<Stmt> parse_for_statement();
    std::shared_ptr<Stmt> parse_return_statement();
    std::shared_ptr<Stmt> parse_break_statement();
    std::shared_ptr<Stmt> parse_continue_statement();
    std::shared_ptr<Stmt> parse_simple_statement();

    std::vector<Token> collect_parenthesized_tokens();
    std::vector<Token> collect_until_top_level_semicolon();
    std::vector<std::vector<Token>> split_top_level(const std::vector<Token>& tokens,
                                                    const std::string& separator) const;
    std::string tokens_to_text(const std::vector<Token>& tokens) const;
    bool looks_like_declaration(const std::vector<Token>& tokens) const;

    std::vector<Token> tokens_;
    std::size_t position_{0};
};

}  // namespace cfglab
