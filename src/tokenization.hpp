#include <string>
#include <vector>
#include <iostream>
#include <experimental/optional>

enum class TokenType {
    _return,
    int_literal,
    semi,
    open_p,
    close_p,
    comma,
    ident,
    var,
    eq,
    plus,
    ast, //asktrick *
    sub,
    f_slash, //division
    greater, // >
    lesser, // <
    open_c,
    close_c,
    _if,
    wif,
    _else,
    _while,
    func,
    give, //equiv to return in cpp
    //_true,
    //_false,
};

bool is_bin_operator(TokenType type) {
    switch (type) {
    case TokenType::plus:
    case TokenType::ast:
        return true;
    default:
        return false;
    }
}

std::optional<int> bin_prec(TokenType type) {
    switch(type) {
    case TokenType::eq:
    case TokenType::greater:
    case TokenType::lesser:
        return 0; // only need to be left and right :)
    case TokenType::plus:
    case TokenType::sub:
        return 1;
    case TokenType::ast:
    case TokenType::f_slash:
        return 2;
    default:
        return {};
    }
}

struct Token {
    TokenType type;
    std::experimental::optional<std::string> value;
};

class Tokenizer {
public:
    inline Tokenizer(const std::string& src) : m_src(std::move(src)) {
        
    }
    inline std::vector<Token> tokenize() {

        std::vector<Token> tokens;
        std::string buffer;
        while(peek().is_value){
            if (std::isalpha(peek().value)) {
                buffer.push_back(consume());
                while (peek().is_value && std::isalnum(peek().value)) {
                    buffer.push_back(consume());
                }
                if (buffer == "ret") {
                    tokens.push_back({.type = TokenType::_return});
                    buffer.clear();
                    continue;
                } else if (buffer == "var") {
                    tokens.push_back({.type = TokenType::var});
                    buffer.clear();
                    continue; 
                } else if (buffer == "if") {
                    tokens.push_back({.type = TokenType::_if});
                    buffer.clear();
                    continue; 
                } else if (buffer == "wif") {
                    tokens.push_back({.type = TokenType::wif});
                    buffer.clear();
                    continue; 
                } else if (buffer == "else") {
                    tokens.push_back({.type = TokenType::_else});
                    buffer.clear();
                    continue; 
                } else if (buffer == "while") {
                    tokens.push_back({.type = TokenType::_while});
                    buffer.clear();
                    continue; 
                } else if (buffer == "func") {
                    tokens.push_back({.type = TokenType::func});
                    buffer.clear();
                    continue; 
                } else if (buffer == "give") {
                    tokens.push_back({.type = TokenType::give});
                    buffer.clear();
                    continue; 
                } else if (buffer == "true") {
                    tokens.push_back({.type = TokenType::int_literal, .value="1"});
                    buffer.clear();
                    continue; 
                } else if (buffer == "false") {
                    tokens.push_back({.type = TokenType::int_literal, .value="0"});
                    buffer.clear();
                    continue; 
                } else {
                    //its an identifier!
                    tokens.push_back({.type = TokenType::ident, .value = buffer});
                    buffer.clear();
                    continue;
                }
                continue;
            }
            else if (std::isdigit(peek().value)) {
                buffer.push_back(consume());
                while (peek().is_value && std::isdigit(peek().value)) {
                    buffer.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_literal, .value=buffer});
                buffer.clear();
                continue;
            }
            else if (peek().value == '-' && peek(1).is_value && peek(1).value == '-' &&
                     peek(2).is_value && peek(2).value == '[' &&
                     peek(3).is_value && peek(3).value == '[') { //multi-line
                consume(); consume(); //consume both dashes
                consume(); consume(); //consume both brackets
                while (peek().is_value) {
                    if (peek().value == ']' && peek(1).is_value && peek(1).value == ']' &&
                     peek(2).is_value && peek(2).value == '-' &&
                     peek(3).is_value && peek(3).value == '-') {
                        consume(); consume(); consume(); consume();
                        break;
                     }
                    consume(); //skip everything until new line :)
                }
            }
            else if (peek().value == '-' && peek(1).is_value && peek(1).value == '-') { //single-line
                consume();
                consume(); //consume both dashes
                while (peek().is_value && peek().value != '\n') {
                    consume(); //skip everything until new line :)
                }
                continue;
            } 
            else if (peek().value == ',') {
                consume();
                tokens.push_back({.type = TokenType::comma});
                continue;
            }
            else if (peek().value == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
                continue;
            }
            else if (peek().value == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
                continue;
            }
            else if (peek().value == '>') {
                consume();
                tokens.push_back({.type = TokenType::greater});
                continue;
            }
            else if (peek().value == '<') {
                consume();
                tokens.push_back({.type = TokenType::lesser});
                continue;
            }
            else if (peek().value == '*') {
                consume();
                tokens.push_back({.type = TokenType::ast});
                continue;
            }
            else if (peek().value == '-') {
                consume();
                tokens.push_back({.type = TokenType::sub});
                continue;
            }
            else if (peek().value == '/') {
                consume();
                tokens.push_back({.type = TokenType::f_slash});
                continue;
            }
            else if (peek().value == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
                continue;
            }
            else if (peek().value == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_p});
                continue;
            }
            else if (peek().value == ')') {
                consume();
                tokens.push_back({.type = TokenType::close_p});
                continue;
            }
            else if (peek().value == '{') {
                consume();
                tokens.push_back({.type = TokenType::open_c});
                continue;
            }
            else if (peek().value == '}') {
                consume();
                tokens.push_back({.type = TokenType::close_c});
                continue;
            }
            else if (std::isspace(peek().value)) {
                consume();
                continue; //Ignore Whitespace
            } else {
                std::cerr << "Unrecognized Token???" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_cur_index = 0;
        return tokens;
    }

private:

    struct val {

        char value;
        bool is_value;
        
    };

    val peek(int count = 0) {
        // Peaks ahead to current + count character in src //
        val retVal;
        
        if (m_cur_index + count >= m_src.length()) {
            retVal.is_value = false;
            retVal.value = '\0';
            return retVal;
        } else {
            retVal.value = m_src.at(m_cur_index + count);
            retVal.is_value = true;
            return retVal;
        }
    };

    inline char consume() {
        return m_src.at(m_cur_index++);
    };

    const std::string m_src;
    int m_cur_index = 0;
};
