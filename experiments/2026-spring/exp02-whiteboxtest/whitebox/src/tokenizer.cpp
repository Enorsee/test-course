#include "tokenizer.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace cfglab {

namespace {

const std::unordered_set<std::string> kKeywords = {
    "if",       "else",   "while",   "do",      "for",      "return",
    "break",    "continue", "int",   "char",    "float",    "double",
    "long",     "short",  "void",    "unsigned", "signed",  "const",
    "static",   "bool",   "struct",  "enum",    "sizeof"
};

bool is_identifier_start(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool is_identifier_part(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool starts_with(const std::string& value, std::size_t position,
                 const std::string& needle) {
    return value.compare(position, needle.size(), needle) == 0;
}

}  // namespace

std::string strip_preprocessor_lines(const std::string& source) {
    std::string result;
    std::size_t line_start = 0;
    while (line_start < source.size()) {
        std::size_t line_end = source.find('\n', line_start);
        if (line_end == std::string::npos) {
            line_end = source.size();
        }
        const std::string line = source.substr(line_start, line_end - line_start);
        const std::size_t non_space = line.find_first_not_of(" \t\r");
        if (non_space == std::string::npos || line[non_space] != '#') {
            result += line;
            if (line_end < source.size()) {
                result.push_back('\n');
            }
        }
        line_start = line_end + 1;
    }
    return result;
}

std::vector<Token> tokenize(const std::string& raw_source) {
    const std::string source = strip_preprocessor_lines(raw_source);
    std::vector<Token> tokens;
    int line = 1;
    int column = 1;

    const auto push_token = [&](TokenKind kind, const std::string& text, int token_line,
                                int token_column) {
        tokens.push_back(Token{kind, text, token_line, token_column});
    };

    std::size_t index = 0;
    while (index < source.size()) {
        const char ch = source[index];

        if (ch == '\n') {
            ++line;
            column = 1;
            ++index;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(ch))) {
            ++column;
            ++index;
            continue;
        }
        if (starts_with(source, index, "//")) {
            while (index < source.size() && source[index] != '\n') {
                ++index;
                ++column;
            }
            continue;
        }
        if (starts_with(source, index, "/*")) {
            index += 2;
            column += 2;
            while (index + 1 < source.size() && !starts_with(source, index, "*/")) {
                if (source[index] == '\n') {
                    ++line;
                    column = 1;
                    ++index;
                } else {
                    ++index;
                    ++column;
                }
            }
            if (index + 1 >= source.size()) {
                throw std::runtime_error("Unterminated block comment");
            }
            index += 2;
            column += 2;
            continue;
        }
        if (is_identifier_start(ch)) {
            const int token_line = line;
            const int token_column = column;
            std::size_t start = index;
            while (index < source.size() && is_identifier_part(source[index])) {
                ++index;
                ++column;
            }
            const std::string text = source.substr(start, index - start);
            const TokenKind kind =
                kKeywords.count(text) > 0 ? TokenKind::Keyword : TokenKind::Identifier;
            push_token(kind, text, token_line, token_column);
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            const int token_line = line;
            const int token_column = column;
            std::size_t start = index;
            while (index < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[index])) ||
                    source[index] == '.' || source[index] == '_')) {
                ++index;
                ++column;
            }
            push_token(TokenKind::Number, source.substr(start, index - start), token_line,
                       token_column);
            continue;
        }
        if (ch == '"' || ch == '\'') {
            const int token_line = line;
            const int token_column = column;
            const char quote = ch;
            std::string literal;
            literal.push_back(ch);
            ++index;
            ++column;
            bool escaped = false;
            while (index < source.size()) {
                const char current = source[index];
                literal.push_back(current);
                ++index;
                if (current == '\n') {
                    ++line;
                    column = 1;
                } else {
                    ++column;
                }
                if (escaped) {
                    escaped = false;
                    continue;
                }
                if (current == '\\') {
                    escaped = true;
                    continue;
                }
                if (current == quote) {
                    break;
                }
            }
            if (literal.back() != quote) {
                throw std::runtime_error("Unterminated string literal");
            }
            push_token(TokenKind::String, literal, token_line, token_column);
            continue;
        }

        const int token_line = line;
        const int token_column = column;
        std::string symbol;
        const std::string two_char =
            index + 1 < source.size() ? source.substr(index, 2) : std::string();
        const std::unordered_set<std::string> multi_char_symbols = {
            "==", "!=", "<=", ">=", "&&", "||", "++", "--", "->", "+=", "-=", "*=",
            "/=", "%=", "<<", ">>"};
        if (!two_char.empty() && multi_char_symbols.count(two_char) > 0) {
            symbol = two_char;
            index += 2;
            column += 2;
        } else {
            symbol.assign(1, ch);
            ++index;
            ++column;
        }
        push_token(TokenKind::Symbol, symbol, token_line, token_column);
    }

    tokens.push_back(Token{TokenKind::End, "<eof>", line, column});
    return tokens;
}

bool is_identifier_like(const Token& token) {
    return token.kind == TokenKind::Identifier || token.kind == TokenKind::Keyword;
}

}  // namespace cfglab
