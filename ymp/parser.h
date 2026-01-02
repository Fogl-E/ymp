#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include <vector>
#include <string>
#include <memory>

struct ParseTreeNode {
    std::string name;
    Token token;
    std::vector<std::shared_ptr<ParseTreeNode>> children;
    int line;
    ParseTreeNode(const std::string& n, const Token& t = Token(), int l = 0);
    void addChild(std::shared_ptr<ParseTreeNode> child);
};

class Parser {
private:
    Lexer& lexer;
    Token currentToken;
    std::vector<std::string> errors;
    void nextToken();
    void error(const std::string& message);
    void match(TokenType expected);
    void synchronizeToStatementEnd();
    bool parseSimpleNumExpr(std::shared_ptr<ParseTreeNode> parent);
    std::shared_ptr<ParseTreeNode> parseBegin();
    std::shared_ptr<ParseTreeNode> parseEnd();
    std::shared_ptr<ParseTreeNode> parseFunctionName();
    std::shared_ptr<ParseTreeNode> parseDescriptions();
    std::shared_ptr<ParseTreeNode> parseOperators();
    std::shared_ptr<ParseTreeNode> parseDescr();
    std::shared_ptr<ParseTreeNode> parseVarList();
    std::shared_ptr<ParseTreeNode> parseType();
    std::shared_ptr<ParseTreeNode> parseOp();
    std::shared_ptr<ParseTreeNode> parseNumExpr();
    std::shared_ptr<ParseTreeNode> parseStringExpr();
    std::shared_ptr<ParseTreeNode> parseSimpleStringExpr();
public:
    Parser(Lexer& l);
    std::shared_ptr<ParseTreeNode> parseFunction();
    bool hasErrors() const;
    const std::vector<std::string>& getErrors() const;
};

#endif