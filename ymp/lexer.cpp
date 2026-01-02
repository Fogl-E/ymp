#include "lexer.h"
#include <iostream>
#include <cctype>

Lexer::Lexer(const std::string& inputFilename) : currentLine(1), currentPos(0) {
    inputFile.open(inputFilename);
    nextChar();
}

Lexer::~Lexer() {
    if (inputFile.is_open()) inputFile.close();
}

void Lexer::nextChar() {
    if (inputFile.get(currentChar)) {
        currentPos++;
    }
    else {
        currentChar = '\0';
    }
}

Token Lexer::parseIdentifier() {
    std::string value;
    int line = currentLine;
    int pos = currentPos;
    while (currentChar != ' ' && currentChar != '\n' && currentChar != '\0' && !isValidSymbol(currentChar)) {
        value += currentChar;
        nextChar();
    }
    if (value == "return") return Token(TokenType::RETURN, value, line, pos);
    if (value == "int") return Token(TokenType::INT, value, line, pos);
    if (value == "char") return Token(TokenType::CHAR, value, line, pos);
    for (int i = 0; i < value.size(); ++i) {
        if (!std::isalpha(value[i])) {
            return Token(TokenType::ERROR, value, line, pos);
        }
    }
    return Token(TokenType::ID, value, line, pos);
}

Token Lexer::parseNumber() {
    std::string value;
    int line = currentLine;
    int pos = currentPos;
    if (isdigit(currentChar)) {
        value += currentChar;
        nextChar();
    }
    if (value == "0" && isdigit(currentChar)) {
        while (isdigit(currentChar)) {
            value += currentChar;
            nextChar();
        }
        return Token(TokenType::ERROR, value, line, pos);
    }
    while (isdigit(currentChar)) {
        value += currentChar;
        nextChar();
    }
    if (isValidSymbol(currentChar)) {
        return Token(TokenType::INT_NUM, value, line, pos);
    }
    if (currentChar != ' ' && currentChar != '\n' && currentChar != '\0' && !isValidSymbol(currentChar)) {
        while (currentChar != ' ' && currentChar != '\n' && currentChar != '\0' && !isValidSymbol(currentChar)) {
            value += currentChar;
            nextChar();
        }
        return Token(TokenType::ERROR, value, line, pos);
    }
    return Token(TokenType::INT_NUM, value, line, pos);
}

Token Lexer::parseString() {
    std::string value;
    int line = currentLine;
    int pos = currentPos;
    nextChar();
    value = '"';
    while (currentChar != '"' && currentChar != '\0' && currentChar != '\n') {
        value += currentChar;
        nextChar();
    }
    if (currentChar == '"') {
        nextChar();
        value = value + '"';
        return Token(TokenType::CHAR_CONST, value, line, pos);
    }
    else {
        return Token(TokenType::ERROR, value, line, pos);
    }
}


bool Lexer::isValidSymbol(char c) {
    return c == '+' || c == '-' || c == '=' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ';' || c == '"' || c == '/' || isspace(c) || c == '\0';
}

Token Lexer::getNextToken() {
    while (isspace(currentChar)) {
        if (currentChar == '\n') {
            currentLine++;
            currentPos = 0;
        }
        nextChar();
    }
    if (currentChar == '\0') {
        return Token(TokenType::END_OF_FILE, "", currentLine, currentPos);
    }
    int line = currentLine;
    int pos = currentPos;
    if (isalpha(currentChar)) {
        return parseIdentifier();
    }
    if (isdigit(currentChar)) {
        return parseNumber();
    }
    if (currentChar == '"') {
        return parseString();
    }
    char ch = currentChar;
    nextChar();
    switch (ch) {
    case '+': return Token(TokenType::PLUS, "+", line, pos);
    case '-': return Token(TokenType::MINUS, "-", line, pos);
    case '=': return Token(TokenType::ASSIGN, "=", line, pos);
    case '(': return Token(TokenType::LPAREN, "(", line, pos);
    case ')': return Token(TokenType::RPAREN, ")", line, pos);
    case '{': return Token(TokenType::LBRACE, "{", line, pos);
    case '}': return Token(TokenType::RBRACE, "}", line, pos);
    case ',': return Token(TokenType::COMMA, ",", line, pos);
    case ';': return Token(TokenType::SEMICOLON, ";", line, pos);
    default:
        std::string value(1, ch);
        while (currentChar != '\0' && !isValidSymbol(currentChar)) {
            value += currentChar;
            nextChar();
        }
        return Token(TokenType::ERROR, value, line, pos);
    }
}