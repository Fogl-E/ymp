#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    FUNCTION, BEGIN, DESCRIPTIONS, OPERATORS, END,
    RETURN, INT, CHAR,
    ID, INT_NUM, CHAR_CONST,
    PLUS, MINUS, ASSIGN,
    LPAREN, RPAREN, LBRACE, RBRACE, COMMA, SEMICOLON,
    END_OF_FILE, ERROR
};

class Token {
public:
    TokenType type;
    std::string value;
    int line;
    int position;
    Token(TokenType t = TokenType::END_OF_FILE, const std::string& v = "", int l = 0, int p = 0);
    std::string getTypeString() const;
};

#endif