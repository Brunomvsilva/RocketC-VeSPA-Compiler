#ifndef SEMANTIC_VISITOR_HPP
#define SEMANTIC_VISITOR_HPP

#include "../AST/ASTNode.hpp"
#include "SymbolTable.hpp"

namespace modern {

class SemanticVisitor : public Visitor {
private:
    SymbolTable symbols;
    int indent = 0;
    void printIndent() { for(int i=0; i<indent; ++i) std::cout << "  "; }

public:
    void visit(BinaryOpNode& n) override {
        printIndent(); std::cout << "BinaryOp [Line " << n.lineNumber << "]" << std::endl;
        indent++;
        if(n.left) n.left->accept(*this);
        if(n.right) n.right->accept(*this);
        indent--;
    }

    void visit(LiteralNode& n) override {
        printIndent(); std::cout << "Literal [Line " << n.lineNumber << "]" << std::endl;
    }

    void visit(IfNode& n) override {
        printIndent(); std::cout << "If [Line " << n.lineNumber << "]" << std::endl;
        indent++;
        if(n.condition) n.condition->accept(*this);
        if(n.thenBlock) n.thenBlock->accept(*this);
        if(n.elseBlock) n.elseBlock->accept(*this);
        indent--;
    }

    void visit(VarDeclNode& n) override {
        printIndent(); std::cout << "VarDecl: " << n.identifier << " [Line " << n.lineNumber << "]" << std::endl;
        symbols.insert(n.identifier, {n.identifier, (VarType_et)n.type, n.lineNumber});
    }

    void visit(UnimplementedNode& n) override {
        printIndent(); std::cout << "(Nó não mapeado: " << n.typeName << " na linha " << n.lineNumber << ")" << std::endl;
    }
};

} 
#endif