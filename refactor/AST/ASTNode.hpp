#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Visitor;
class BinaryOpNode;
class LiteralNode;
class IfNode;
class VarDeclNode;
class UnimplementedNode;

class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visit(BinaryOpNode& n) = 0;
    virtual void visit(LiteralNode& n) = 0;
    virtual void visit(IfNode& n) = 0;
    virtual void visit(VarDeclNode& n) = 0;
    virtual void visit(UnimplementedNode& n) = 0;
};

class ASTNode {
public:
    int lineNumber;
    ASTNode(int line) : lineNumber(line) {}
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& v) = 0;
};

class ExpressionNode : public ASTNode {
public:
    int nodeVarType;
    ExpressionNode(int line) : ASTNode(line), nodeVarType(0) {}
};

class BinaryOpNode : public ExpressionNode {
public:
    int op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
    
    BinaryOpNode(int line, int op, std::unique_ptr<ExpressionNode> l, std::unique_ptr<ExpressionNode> r) 
        : ExpressionNode(line), op(op), left(std::move(l)), right(std::move(r)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class LiteralNode : public ExpressionNode {
public:
    int literalType;
    union {
        int iVal;
        float fVal;
        char cVal;
    } value;

    LiteralNode(int line, int type, int val) : ExpressionNode(line), literalType(type) { value.iVal = val; }
    LiteralNode(int line, int type, float val) : ExpressionNode(line), literalType(type) { value.fVal = val; }

    void accept(Visitor& v) override { v.visit(*this); }
};

class StatementNode : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class IfNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> thenBlock;
    std::unique_ptr<ASTNode> elseBlock;
    
    IfNode(int line, std::unique_ptr<ExpressionNode> cond, std::unique_ptr<ASTNode> thenB, std::unique_ptr<ASTNode> elseB = nullptr)
        : StatementNode(line), condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class VarDeclNode : public ASTNode {
public:
    std::string identifier;
    int type;
    
    VarDeclNode(int line, std::string id, int t) : ASTNode(line), identifier(id), type(t) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};


class UnimplementedNode : public ASTNode {
public:
    std::string typeName;
    UnimplementedNode(int line, std::string name) : ASTNode(line), typeName(name) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

#endif