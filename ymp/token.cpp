#include "token.h"

Token::Token(TokenType t, const std::string& v, int l, int p)
    : type(t), value(v), line(l), position(p) {
}

std::string Token::getTypeString() const {
    switch (type) {
    case TokenType::RETURN: return "RETURN";
    case TokenType::INT: return "INT";
    case TokenType::CHAR: return "CHAR";
    case TokenType::ID: return "ID";
    case TokenType::INT_NUM: return "INT_NUM";
    case TokenType::CHAR_CONST: return "CHAR_CONST";
    case TokenType::PLUS: return "PLUS";
    case TokenType::MINUS: return "MINUS";
    case TokenType::ASSIGN: return "ASSIGN";
    case TokenType::LPAREN: return "LPAREN";
    case TokenType::RPAREN: return "RPAREN";
    case TokenType::LBRACE: return "LBRACE";
    case TokenType::RBRACE: return "RBRACE";
    case TokenType::COMMA: return "COMMA";
    case TokenType::SEMICOLON: return "SEMICOLON";
    case TokenType::END_OF_FILE: return "END_OF_FILE";
    case TokenType::ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}
