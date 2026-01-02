#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <fstream>
#include <string>

class Lexer {
private:
    std::ifstream inputFile;
    int currentLine;
    int currentPos;
    char currentChar;
    void nextChar();
    Token parseIdentifier();
    Token parseNumber();
    Token parseString();
    bool isValidSymbol(char c);
public:
    Lexer(const std::string& inputFilename);
    ~Lexer();
    Token getNextToken();
};

#endif