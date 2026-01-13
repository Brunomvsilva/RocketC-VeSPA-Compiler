#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include "../AST/ASTNode.hpp"
#include <memory>

extern "C" {
    struct TreeNode;
}

std::unique_ptr<ASTNode> liftAST(struct TreeNode* cNode);

#endif