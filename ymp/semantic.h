#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"
#include "hashtable.h"  
#include <string>
#include <vector>
#include <fstream>

enum class SymbolType {
    INT_TYPE,
    CHAR_TYPE,
    FUNCTION_TYPE,
    UNDEFINED
};

struct SymbolInfo {
    std::string name;
    SymbolType type;
    int line;
    bool isFunction;
    SymbolType returnType;
    SymbolInfo(const std::string& n = "", SymbolType t = SymbolType::UNDEFINED, int l = 0, bool isFunc = false, SymbolType retType = SymbolType::UNDEFINED);
};

class SemanticAnalyzer {
private:
    HashTable symbolTable; 
    std::vector<SymbolInfo> symbolInfoList;  
    std::vector<std::string> errors;
    SymbolType currentFunctionReturnType;
    std::string currentFunctionName;
    void addError(const std::string& message, int line);
    SymbolType getTypeFromToken(TokenType tokenType);
    void analyzeNode(std::shared_ptr<ParseTreeNode> node);
    void analyzeFunction(std::shared_ptr<ParseTreeNode> funcNode);
    void analyzeBegin(std::shared_ptr<ParseTreeNode> beginNode);
    void analyzeDescriptions(std::shared_ptr<ParseTreeNode> descNode);
    void analyzeDescr(std::shared_ptr<ParseTreeNode> descrNode);
    void analyzeVarList(std::shared_ptr<ParseTreeNode> varListNode, SymbolType type);
    void analyzeOperators(std::shared_ptr<ParseTreeNode> operatorsNode);
    void analyzeOp(std::shared_ptr<ParseTreeNode> opNode);
    void checkNumExprForAssignment(std::shared_ptr<ParseTreeNode> opNode, const SymbolInfo& targetVar, int assignmentLine);
    void analyzeEnd(std::shared_ptr<ParseTreeNode> endNode);
    SymbolType checkNumExpr(std::shared_ptr<ParseTreeNode> node);
    SymbolType checkStringExpr(std::shared_ptr<ParseTreeNode> node);
    SymbolType checkSimpleNumExpr(std::shared_ptr<ParseTreeNode> node);
    int findSymbolIndex(const std::string& name) const;
    const SymbolInfo* findSymbolInfo(const std::string& name) const;
    SymbolInfo* findSymbolInfo(const std::string& name);
    void addSymbolInfo(const SymbolInfo& info);

    void generatePostfixForNode(std::shared_ptr<ParseTreeNode> node, std::ofstream& outFile);

public:
    SemanticAnalyzer();
    void analyze(std::shared_ptr<ParseTreeNode> root);
    bool hasErrors() const;
    const std::vector<std::string>& getErrors() const;
    void generatePostfix(std::shared_ptr<ParseTreeNode> node, std::ofstream& outFile);
};

#endif