#include <iostream>
#include <cstdio>
#include "Bridge/ASTBuilder.hpp"
#include "Semantics/SemanticVisitor.hpp"

extern "C" {
    #ifndef YYTOKENTYPE_GUARD
    #define YYTOKENTYPE_GUARD
    enum yytokentype { DUMMY_T = 0 };
    #endif
    
    #include "../OldCompiler/Parser/Parser.h"
    #include "../OldCompiler/Util/TreeNode.h"
    extern FILE* yyin;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            std::cerr << "Erro: Não foi possível abrir o ficheiro " << argv[1] << std::endl;
            return 1;
        }
    }

    TreeNode_st* cRoot = nullptr;
    std::cout << "--- [Refactor] A invocar Parser (C) ---" << std::endl;
    
    if (executeParser((TreeNode_st**)&cRoot) != 0 || !cRoot) {
        std::cerr << "Erro: Falha na análise sintática." << std::endl;
        return 1;
    }

    auto cppRoot = liftAST(cRoot);
    
    std::cout << "--- [Refactor] Análise Semântica (Visitor) ---" << std::endl;
    modern::SemanticVisitor visitor;
    cppRoot->accept(visitor); 

    std::cout << "--- [Refactor] Concluído com sucesso ---" << std::endl;
    
    if (yyin) fclose(yyin);
    return 0;
}