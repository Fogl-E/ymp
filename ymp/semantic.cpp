#include "semantic.h"
#include <iostream>
#include <sstream>

SymbolInfo::SymbolInfo(const std::string& n, SymbolType t, int l, bool isFunc, SymbolType retType) : name(n), type(t), line(l), isFunction(isFunc), returnType(retType) {
}

SemanticAnalyzer::SemanticAnalyzer() : currentFunctionReturnType(SymbolType::UNDEFINED), currentFunctionName("") {
}

void SemanticAnalyzer::addError(const std::string& message, int line) {
    std::stringstream ss;
    ss << "Semantic error at line " << line << ": " << message;
    errors.push_back(ss.str());
}

SymbolType SemanticAnalyzer::getTypeFromToken(TokenType tokenType) {
    switch (tokenType) {
    case TokenType::INT: return SymbolType::INT_TYPE;
    case TokenType::CHAR: return SymbolType::CHAR_TYPE;
    default: return SymbolType::UNDEFINED;
    }
}

void SemanticAnalyzer::analyze(std::shared_ptr<ParseTreeNode> root) {
    if (!root) return;
    analyzeNode(root);
}

void SemanticAnalyzer::analyzeNode(std::shared_ptr<ParseTreeNode> node) {
    if (!node) return;
    if (node->name == "Function") {
        analyzeFunction(node);
    }
    for (const auto& child : node->children) {
        analyzeNode(child);
    }
}
void SemanticAnalyzer::analyzeFunction(std::shared_ptr<ParseTreeNode> funcNode) {
    currentFunctionReturnType = SymbolType::UNDEFINED;
    currentFunctionName = "";
    for (const auto& child : funcNode->children) {
        if (child->name == "Begin") {
            analyzeBegin(child);
        }
        else if (child->name == "Descriptions") {
            analyzeDescriptions(child);
        }
        else if (child->name == "Operators") {
            analyzeOperators(child);
        }
        else if (child->name == "End") {
            analyzeEnd(child);
        }
    }
}

void SemanticAnalyzer::analyzeBegin(std::shared_ptr<ParseTreeNode> beginNode) {
    if (beginNode->children.size() >= 2) {
        auto typeNode = beginNode->children[0];
        if (typeNode->name == "Type" && !typeNode->children.empty()) {
            TokenType tokenType = typeNode->children[0]->token.type;
            currentFunctionReturnType = getTypeFromToken(tokenType);
        }
        auto nameNode = beginNode->children[1];
        if (nameNode->name == "FunctionName" && !nameNode->children.empty()) {
            currentFunctionName = nameNode->children[0]->token.value;
            int line = nameNode->children[0]->token.line;
            if (symbolTable.find(currentFunctionName) != symbolTable.end()) {
                addError("Function '" + currentFunctionName + "' already declared", line);
            }
            else {
                symbolTable[currentFunctionName] = SymbolInfo(currentFunctionName, SymbolType::FUNCTION_TYPE, line, true, currentFunctionReturnType);
            }
        }
    }
}

void SemanticAnalyzer::analyzeDescriptions(std::shared_ptr<ParseTreeNode> descNode) {
    for (const auto& descr : descNode->children) {
        if (descr->name == "Descr") {
            analyzeDescr(descr);
        }
    }
}

void SemanticAnalyzer::analyzeDescr(std::shared_ptr<ParseTreeNode> descrNode) {
    if (descrNode->children.size() >= 2) {
        auto typeNode = descrNode->children[0];
        auto varListNode = descrNode->children[1];

        SymbolType varType = SymbolType::UNDEFINED;
        if (typeNode->name == "Type" && !typeNode->children.empty()) {
            TokenType tokenType = typeNode->children[0]->token.type;
            varType = getTypeFromToken(tokenType);
        }
        analyzeVarList(varListNode, varType);
    }
}

void SemanticAnalyzer::analyzeVarList(std::shared_ptr<ParseTreeNode> varListNode, SymbolType type) {
    for (const auto& child : varListNode->children) {
        if (child->name == "Id") {
            std::string varName = child->token.value;
            int line = child->token.line;
            if (symbolTable.find(varName) != symbolTable.end()) {
                auto existing = symbolTable[varName];
                std::string existingWhat = existing.isFunction ? "function" : "variable";
                addError("'" + varName + "' already declared as " + existingWhat + " at line " + std::to_string(existing.line), line);
            }
            else {
                symbolTable[varName] = SymbolInfo(varName, type, line);
            }
        }
    }
}

void SemanticAnalyzer::analyzeOperators(std::shared_ptr<ParseTreeNode> operatorsNode) {
    for (const auto& op : operatorsNode->children) {
        if (op->name == "Op") {
            analyzeOp(op);
        }
    }
}

void SemanticAnalyzer::analyzeOp(std::shared_ptr<ParseTreeNode> opNode) {
    if (opNode->children.size() >= 2) {
        auto idNode = opNode->children[0];
        if (idNode->name == "Id") {
            std::string varName = idNode->token.value;
            int line = idNode->token.line;

            // Проверка объявления переменной
            if (symbolTable.find(varName) == symbolTable.end()) {
                addError("Undeclared variable '" + varName + "'", line);
                return;
            }

            SymbolInfo& varInfo = symbolTable[varName];
            auto exprNode = opNode->children[1];

            if (exprNode->name == "NumExpr") {
                // Анализ числового выражения с рекурсивным обходом
                checkNumExprForAssignment(exprNode, varInfo, line);
                checkNumExpr(exprNode); // Дополнительная проверка выражения
            }
            else if (exprNode->name == "StringExpr") {
                checkStringExpr(exprNode);
                if (varInfo.type == SymbolType::INT_TYPE) {
                    addError("cannot assign char to int variable '" + varName + "'", line);
                }
            }
        }
    }
}

// Рекурсивная функция для проверки совместимости типов в присваивании
void SemanticAnalyzer::checkNumExprForAssignment(std::shared_ptr<ParseTreeNode> node,
    const SymbolInfo& targetVar,
    int assignmentLine) {
    if (!node) return;

    // Проверка идентификаторов в выражении
    if (node->name == "Id") {
        std::string exprVarName = node->token.value;
        if (symbolTable.find(exprVarName) != symbolTable.end()) {
            SymbolInfo& exprVarInfo = symbolTable[exprVarName];

            // Проверка совместимости типов
            if (targetVar.type == SymbolType::INT_TYPE && exprVarInfo.type == SymbolType::CHAR_TYPE) {
                addError("cannot assign char '" + exprVarName + "' to int '" + targetVar.name + "'",
                    assignmentLine);
            }
            else if (targetVar.type == SymbolType::CHAR_TYPE && exprVarInfo.type == SymbolType::INT_TYPE) {
                addError("cannot assign int '" + exprVarName + "' to char '" + targetVar.name + "'",
                    assignmentLine);
            }
        }
    }
    // Проверка констант для char переменных
    else if (node->name == "Const" && targetVar.type == SymbolType::CHAR_TYPE) {
        addError("cannot assign integer '" + node->token.value + "' to char '" + targetVar.name + "'",
            assignmentLine);
    }

    // Рекурсивный обход дочерних узлов
    for (const auto& child : node->children) {
        checkNumExprForAssignment(child, targetVar, assignmentLine);
    }
}
SymbolType SemanticAnalyzer::checkNumExpr(std::shared_ptr<ParseTreeNode> node) {
    for (const auto& child : node->children) {
        if (child->name == "SimpleNumExpr") {
            if (checkSimpleNumExpr(child) == SymbolType::UNDEFINED) {
                return SymbolType::UNDEFINED;
            }
        }
    }
    return SymbolType::INT_TYPE;
}

SymbolType SemanticAnalyzer::checkSimpleNumExpr(std::shared_ptr<ParseTreeNode> node) {
    for (const auto& child : node->children) {
        if (child->name == "Id") {
            std::string varName = child->token.value;
            if (symbolTable.find(varName) == symbolTable.end()) {
                addError("Undeclared variable '" + varName + "'", child->token.line);
                return SymbolType::UNDEFINED;
            }
            SymbolInfo& varInfo = symbolTable[varName];
            if (varInfo.type != SymbolType::INT_TYPE) {
                addError("Variable '" + varName + "' must be integer type in numeric expression", child->token.line);
                return SymbolType::UNDEFINED;
            }
            return SymbolType::INT_TYPE;
        }
        else if (child->name == "Const") {
            return SymbolType::INT_TYPE;
        }
        else if (child->name == "NumExpr") {
            return checkNumExpr(child);
        }
    }
    return SymbolType::UNDEFINED;
}

SymbolType SemanticAnalyzer::checkStringExpr(std::shared_ptr<ParseTreeNode> node) {
    for (const auto& child : node->children) {
        if (child->name == "SimpleStringExpr") {
            for (const auto& subChild : child->children) {
                if (subChild->name == "char_const") {
                    return SymbolType::CHAR_TYPE;
                }
            }
        }
        else if (child->name == "Plus") {
            return SymbolType::CHAR_TYPE;
        }
    }
    return SymbolType::UNDEFINED;
}

void SemanticAnalyzer::analyzeEnd(std::shared_ptr<ParseTreeNode> endNode) {
    if (!endNode->children.empty()) {
        auto returnIdNode = endNode->children[0];
        if (returnIdNode->name == "Id") {
            std::string varName = returnIdNode->token.value;
            int line = returnIdNode->token.line;
            if (symbolTable.find(varName) == symbolTable.end()) {
                addError("Undeclared variable '" + varName + "' in return statement", line);
                return;
            }
            SymbolInfo& varInfo = symbolTable[varName];
            if (varInfo.isFunction) {
                addError("Cannot return function '" + varName + "'", line);
                return;
            }
            if (varInfo.type != currentFunctionReturnType) {
                std::string funcType = (currentFunctionReturnType == SymbolType::INT_TYPE) ? "int" : "char";
                std::string varType = (varInfo.type == SymbolType::INT_TYPE) ? "int" : "char";
                addError("function returns " + funcType + " but variable is " + varType, line);
            }
        }
    }
}

bool SemanticAnalyzer::hasErrors() const {
    return !errors.empty();
}

const std::vector<std::string>& SemanticAnalyzer::getErrors() const {
    return errors;
}

void SemanticAnalyzer::generatePostfixForNode(std::shared_ptr<ParseTreeNode> node, std::ofstream& outFile) {
    if (!node) return;
    if (node->name == "Id") {
        outFile << node->token.value << " ";
    }
    else if (node->name == "Const") {
        outFile << node->token.value << " ";
    }
    else if (node->name == "char_const") {
        std::string value = node->token.value;
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            outFile << "\"" << value.substr(1, value.size() - 2) << "\" ";
        }
        else {
            outFile << value << " ";
        }
    }
    else if (node->name == "Plus" || node->name == "Minus") {
        outFile << node->token.value << " ";
    }
    else if (node->name == "NumExpr") {
        if (node->children.size() == 3) {
            generatePostfixForNode(node->children[0], outFile);
            generatePostfixForNode(node->children[2], outFile);
            generatePostfixForNode(node->children[1], outFile);
        }
        else {
            for (const auto& child : node->children) {
                generatePostfixForNode(child, outFile);
            }
        }
    }
    else if (node->name == "StringExpr") {
        if (node->children.size() == 3) {
            generatePostfixForNode(node->children[0], outFile);
            generatePostfixForNode(node->children[2], outFile);
            outFile << "+ ";
        }
        else {
            for (const auto& child : node->children) {
                generatePostfixForNode(child, outFile);
            }
        }
    }
    else {
        for (const auto& child : node->children) {
            generatePostfixForNode(child, outFile);
        }
    }
}

void SemanticAnalyzer::generatePostfix(std::shared_ptr<ParseTreeNode> node, std::ofstream& outFile) {
    if (!node) return;
    if (node->name == "Function") {
        outFile << "\n=== POSTFIX NOTATION ===" << std::endl;
        for (const auto& child : node->children) {
            if (child->name == "Descriptions") {
                for (const auto& descr : child->children) {
                    if (descr->name == "Descr" && descr->children.size() >= 2) {
                        auto typeNode = descr->children[0];
                        auto varListNode = descr->children[1];
                        std::string typeStr = "int";
                        if (typeNode->name == "Type" && !typeNode->children.empty()) {
                            auto typeToken = typeNode->children[0]->token.value;
                            typeStr = (typeToken == "char") ? "char" : "int";
                        }
                        int varCount = 0;
                        std::stringstream varsStream;
                        for (const auto& var : varListNode->children) {
                            if (var->name == "Id") {
                                if (varCount > 0) varsStream << " ";
                                varsStream << var->token.value;
                                varCount++;
                            }
                        }
                        if (varCount > 0) {
                            outFile << typeStr << " " << varsStream.str() << " " << varCount + 1 << " decl" << std::endl;
                        }
                    }
                }
            }
        }
        for (const auto& child : node->children) {
            if (child->name == "Operators") {
                for (const auto& op : child->children) {
                    if (op->name == "Op" && op->children.size() >= 2) {
                        generatePostfixForNode(op->children[1], outFile);
                        if (op->children[0]->name == "Id") {
                            outFile << op->children[0]->token.value << " =" << std::endl;
                        }
                    }
                }
            }
        }
        for (const auto& child : node->children) {
            if (child->name == "End") {
                if (!child->children.empty() && child->children[0]->name == "Id") {
                    outFile << child->children[0]->token.value << " RETURN" << std::endl;
                }
            }
        }
    }
}