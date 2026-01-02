#include "lexer.h"
#include "hashtable.h"
#include "parser.h"
#include "semantic.h"
#include <iostream>
#include <fstream>

int main() {
    Lexer lexer("input.txt");
    HashTable hashTable;
    Token token = lexer.getNextToken();
    int tokenCount = 0;
    while (token.type != TokenType::END_OF_FILE) {
        tokenCount++;
        int index = hashTable.insert(token);
        token = lexer.getNextToken();
    }
    std::ofstream outFile("output.txt");
    Lexer parserLexer("input.txt");
    Parser parser(parserLexer);
    auto syntaxTree = parser.parseFunction();
    if (parser.hasErrors()) {
        outFile << "SYNTAX ERRORS:" << std::endl;
        for (const auto& error : parser.getErrors()) {
            outFile << error << std::endl;
        }
    }
    else {
        outFile << "No syntax errors found." << std::endl;
    }
    if (!parser.hasErrors()) {
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyze(syntaxTree);
        if (semanticAnalyzer.hasErrors()) {
            outFile << "SEMANTIC ERRORS:" << std::endl;
            for (const auto& error : semanticAnalyzer.getErrors()) {
                outFile << error << std::endl;
            }
        }
        else {
            outFile << "No semantic errors found." << std::endl;
        }
        semanticAnalyzer.generatePostfix(syntaxTree, outFile);
    }
    outFile.close();
    return 0;
}