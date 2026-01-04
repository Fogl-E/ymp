#include "parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

ParseTreeNode::ParseTreeNode(const std::string& n, const Token& t, int l) : name(n), token(t), line(l) {}

void ParseTreeNode::addChild(std::shared_ptr<ParseTreeNode> child) {
    children.push_back(child);
}

Parser::Parser(Lexer& l) : lexer(l) {
    nextToken();
}

void Parser::nextToken() {
    currentToken = lexer.getNextToken();
}

void Parser::error(const std::string& message) {
    std::stringstream s;
    s << "Syntax error at line " << currentToken.line << ", position " << currentToken.position << ": " << message;
    errors.push_back(s.str());
}

void Parser::synchronizeToStatementEnd() {
    while (currentToken.type != TokenType::SEMICOLON && currentToken.type != TokenType::END_OF_FILE && currentToken.type != TokenType::RETURN && currentToken.type != TokenType::RBRACE && currentToken.type != TokenType::ID) {
        nextToken();
    }
    if (currentToken.type == TokenType::SEMICOLON) {
        nextToken();
    }
}

void Parser::match(TokenType expected) {
    if (currentToken.type == expected) {
        nextToken();
    }
    else {
        error("Expected " + Token(expected).getTypeString() + " but found '" + currentToken.value + "'");
        if (expected == TokenType::SEMICOLON || expected == TokenType::RBRACE || expected == TokenType::RPAREN) {
            synchronizeToStatementEnd();
        }
    }
}

std::shared_ptr<ParseTreeNode> Parser::parseFunction() {
    auto node = std::make_shared<ParseTreeNode>("Function", Token(), currentToken.line);
    node->addChild(parseBegin());
    node->addChild(parseDescriptions());
    node->addChild(parseOperators());
    node->addChild(parseEnd());
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseBegin() {
    auto node = std::make_shared<ParseTreeNode>("Begin", Token(), currentToken.line);
    node->addChild(parseType());
    node->addChild(parseFunctionName());
    match(TokenType::LPAREN);
    match(TokenType::RPAREN);
    match(TokenType::LBRACE);
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseEnd() {
    auto node = std::make_shared<ParseTreeNode>("End", Token(), currentToken.line);
    if (currentToken.type == TokenType::RETURN) {
        match(TokenType::RETURN);
    }
    else {
        error("Expected RETURN but found '" + currentToken.value + "'");
        return node;
    }
    if (currentToken.type == TokenType::ID) {
        auto idNode = std::make_shared<ParseTreeNode>("Id", currentToken, currentToken.line);
        node->addChild(idNode);
        match(TokenType::ID);
    }
    else {
        error("Expected identifier after return");
    }
    match(TokenType::SEMICOLON);
    match(TokenType::RBRACE);
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseFunctionName() {
    auto node = std::make_shared<ParseTreeNode>("FunctionName", Token(), currentToken.line);
    if (currentToken.type == TokenType::ID) {
        auto idNode = std::make_shared<ParseTreeNode>("Id", currentToken, currentToken.line);
        node->addChild(idNode);
        match(TokenType::ID);
    }
    else {
        error("Expected function name identifier");
    }
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseDescriptions() {
    auto node = std::make_shared<ParseTreeNode>("Descriptions", Token(), currentToken.line);
    while (currentToken.type == TokenType::INT || currentToken.type == TokenType::CHAR) {
        node->addChild(parseDescr());
    }
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseOperators() {
    auto node = std::make_shared<ParseTreeNode>("Operators", Token(), currentToken.line);
    while (currentToken.type == TokenType::ID ||
        currentToken.type == TokenType::ASSIGN ||
        currentToken.type == TokenType::RETURN) {
        if (currentToken.type == TokenType::RETURN) {
            break;
        }
        node->addChild(parseOp());
        if (hasErrors() && currentToken.type == TokenType::RETURN) {
            break;
        }
    }
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseDescr() {
    auto node = std::make_shared<ParseTreeNode>("Descr", Token(), currentToken.line);
    node->addChild(parseType());
    node->addChild(parseVarList());
    match(TokenType::SEMICOLON);
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseVarList() {
    auto node = std::make_shared<ParseTreeNode>("VarList", Token(), currentToken.line);
    if (currentToken.type == TokenType::ID) {
        auto idNode = std::make_shared<ParseTreeNode>("Id", currentToken, currentToken.line);
        node->addChild(idNode);
        match(TokenType::ID);
    }
    else {
        error("Expected identifier in variable list");
        return node;
    }
    while (currentToken.type == TokenType::COMMA) {
        match(TokenType::COMMA);
        if (currentToken.type == TokenType::ID) {
            auto idNode = std::make_shared<ParseTreeNode>("Id", currentToken, currentToken.line);
            node->addChild(idNode);
            match(TokenType::ID);
        }
        else {
            error("Expected identifier after comma");
        }
    }
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseType() {
    auto node = std::make_shared<ParseTreeNode>("Type", Token(), currentToken.line);
    if (currentToken.type == TokenType::INT) {
        auto typeNode = std::make_shared<ParseTreeNode>("int", currentToken, currentToken.line);
        node->addChild(typeNode);
        match(TokenType::INT);
    }
    else if (currentToken.type == TokenType::CHAR) {
        auto typeNode = std::make_shared<ParseTreeNode>("char", currentToken, currentToken.line);
        node->addChild(typeNode);
        match(TokenType::CHAR);
    }
    else if (currentToken.type == TokenType::ID) {
        error("Unknown type '" + currentToken.value);
        auto errorNode = std::make_shared<ParseTreeNode>("ERROR", currentToken, currentToken.line);
        node->addChild(errorNode);
        match(TokenType::ID);
    }
    else {
        error("Expected type (int or char)");
    }

    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseOp() {
    auto node = std::make_shared<ParseTreeNode>("Op", Token(), currentToken.line);
    if (currentToken.type == TokenType::ID) {
        auto idNode = std::make_shared<ParseTreeNode>("Id", currentToken, currentToken.line);
        node->addChild(idNode);
        match(TokenType::ID);
        if (currentToken.type == TokenType::ASSIGN) {
            match(TokenType::ASSIGN);
            if (currentToken.type == TokenType::CHAR_CONST) {
                node->addChild(parseStringExpr());
            }
            else {
                node->addChild(parseNumExpr());
            }
            if (hasErrors()) {
                synchronizeToStatementEnd();
            }
            else {
                match(TokenType::SEMICOLON);
            }
        }
        else {
            error("Expected '=' in operator");
            synchronizeToStatementEnd();
        }
    }
    else {
        error("Expected identifier at start of operator but found '" + currentToken.value + "'");
        if (currentToken.type == TokenType::ASSIGN) {
            error("Missing identifier before '='");
            match(TokenType::ASSIGN);
            if (currentToken.type == TokenType::CHAR_CONST) {
                node->addChild(parseStringExpr());
            }
            else {
                node->addChild(parseNumExpr());
            }
            if (hasErrors()) {
                synchronizeToStatementEnd();
            }
            else {
                match(TokenType::SEMICOLON);
            }
        }
        else {
            synchronizeToStatementEnd();
        }
    }

    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseNumExpr() {
    auto node = std::make_shared<ParseTreeNode>("NumExpr", Token(), currentToken.line);
    if (parseSimpleNumExpr(node)) {
        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
            auto opNode = std::make_shared<ParseTreeNode>(currentToken.type == TokenType::PLUS ? "Plus" : "Minus", currentToken, currentToken.line);
            node->addChild(opNode);
            if (currentToken.type == TokenType::PLUS) {
                match(TokenType::PLUS);
            }
            else {
                match(TokenType::MINUS);
            }

            if (!parseSimpleNumExpr(node)) {
                error("Missing operand after '" + opNode->token.value + "' operator");
                break;
            }
        }
    }
    return node;
}

bool Parser::parseSimpleNumExpr(std::shared_ptr<ParseTreeNode> parent) {
    if (currentToken.type == TokenType::ID) {
        auto idNode = std::make_shared<ParseTreeNode>("Id", currentToken, currentToken.line);
        parent->addChild(idNode);
        match(TokenType::ID);
        return true;
    }
    else if (currentToken.type == TokenType::INT_NUM) {
        auto constNode = std::make_shared<ParseTreeNode>("Const", currentToken, currentToken.line);
        parent->addChild(constNode);
        match(TokenType::INT_NUM);
        return true;
    }
    else if (currentToken.type == TokenType::LPAREN) {
        match(TokenType::LPAREN);
        parent->addChild(parseNumExpr());
        if (currentToken.type == TokenType::RPAREN) {
            match(TokenType::RPAREN);
        }
        else {
            error("Expected ')' after expression");
        }
        return true;
    }
    else if (currentToken.type == TokenType::ERROR) {
        auto errorNode = std::make_shared<ParseTreeNode>("ERROR", currentToken, currentToken.line);
        parent->addChild(errorNode);
        error("Invalid token '" + currentToken.value + "' in numeric expression");
        match(TokenType::ERROR);
        return false;
    }
    else {
        error("Expected identifier, constant or '(' in numeric expression");
        return false;
    }
}

std::shared_ptr<ParseTreeNode> Parser::parseStringExpr() {
    auto node = std::make_shared<ParseTreeNode>("StringExpr", Token(), currentToken.line);
    node->addChild(parseSimpleStringExpr());
    while (currentToken.type == TokenType::PLUS) {
        auto plusNode = std::make_shared<ParseTreeNode>("Plus", currentToken, currentToken.line);
        node->addChild(plusNode);
        match(TokenType::PLUS);
        node->addChild(parseSimpleStringExpr());
    }
    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseSimpleStringExpr() {
    auto node = std::make_shared<ParseTreeNode>("SimpleStringExpr", Token(), currentToken.line);
    if (currentToken.type == TokenType::CHAR_CONST) {
        auto stringNode = std::make_shared<ParseTreeNode>("char_const", currentToken, currentToken.line);
        node->addChild(stringNode);
        match(TokenType::CHAR_CONST);
    }
    else if (currentToken.type == TokenType::ERROR) {
        auto errorNode = std::make_shared<ParseTreeNode>("ERROR", currentToken, currentToken.line);
        node->addChild(errorNode);
        error("Invalid token '" + currentToken.value + "' in string expression");
        match(TokenType::ERROR);
    }
    else {
        error("Expected string constant");
    }
    return node;
}
bool Parser::hasErrors() const {
    return !errors.empty();
}
const std::vector<std::string>& Parser::getErrors() const {
    return errors;
}

