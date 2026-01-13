#include "ASTBuilder.hpp"

extern "C" {
    #ifndef YYTOKENTYPE_GUARD
    #define YYTOKENTYPE_GUARD
    enum yytokentype { DUMMY_T = 0 };
    #endif
    #include "../../OldCompiler/Util/Globals.h"
    #include "../../OldCompiler/Util/TreeNode.h"
}

std::unique_ptr<ASTNode> liftAST(TreeNode_st* cNode) {
    if (!cNode) return nullptr;

    switch(cNode->nodeType) {
        case NODE_INTEGER:
        case NODE_FLOAT:
            return std::make_unique<LiteralNode>(cNode->lineNumber, (int)cNode->nodeType, (int)cNode->nodeData.dVal);

        case NODE_VAR_DECLARATION:
            return std::make_unique<VarDeclNode>(cNode->lineNumber, cNode->nodeData.sVal ? cNode->nodeData.sVal : "unknown", (int)cNode->nodeVarType);

        case NODE_OPERATOR: {
            auto l = liftAST(&cNode->pChilds[0]);
            auto r = liftAST(&cNode->pChilds[1]);
            auto left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>(l.release()));
            auto right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>(r.release()));
            return std::make_unique<BinaryOpNode>(cNode->lineNumber, (int)cNode->nodeData.dVal, std::move(left), std::move(right));
        }

        case NODE_IF: {
            auto cond_raw = liftAST(&cNode->pChilds[0]);
            auto cond = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>(cond_raw.release()));
            return std::make_unique<IfNode>(cNode->lineNumber, std::move(cond), liftAST(&cNode->pChilds[1]), (cNode->childNumber > 2) ? liftAST(&cNode->pChilds[2]) : nullptr);
        }

        //Unknown Node Handle
        default:
            if (cNode->childNumber > 0 && &cNode->pChilds[0] != nullptr) {
                return liftAST(&cNode->pChilds[0]);
            }
            return std::make_unique<UnimplementedNode>(cNode->lineNumber, "UnknownNode");
    }
}