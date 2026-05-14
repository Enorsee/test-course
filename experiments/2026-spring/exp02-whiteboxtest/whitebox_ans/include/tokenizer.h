#pragma once

#include <string>
#include <vector>

namespace cfglab {

enum class TokenKind {
    Identifier,
    Number,
    String,
    Symbol,
    Keyword,
    End
};

struct Token {
    TokenKind kind{TokenKind::End};
    std::string text;
    int line{1};
    int column{1};
};

std::string strip_preprocessor_lines(const std::string& source);
std::vector<Token> tokenize(const std::string& source);
bool is_identifier_like(const Token& token);

}  // namespace cfglab
