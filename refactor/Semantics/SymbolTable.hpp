#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <unordered_map>
#include <string>

extern "C" {
    #ifndef YYTOKENTYPE_GUARD
    #define YYTOKENTYPE_GUARD
    enum yytokentype { DUMMY_T = 0 };
    #endif
    #include "../../OldCompiler/Util/Globals.h"
}

namespace modern {

struct SymbolEntry {
    std::string name;
    VarType_et type;
    int lineDefined;
};

class SymbolTable {
private:
    std::unordered_map<std::string, SymbolEntry> table;
    SymbolTable* parentScope;

public:
    SymbolTable(SymbolTable* parent = nullptr) : parentScope(parent) {}

    bool insert(const std::string& name, SymbolEntry entry) {
        if (table.find(name) != table.end()) return false;
        table[name] = entry;
        return true;
    }

    SymbolEntry* lookup(const std::string& name) {
        if (table.count(name)) return &table[name];
        if (parentScope) return parentScope->lookup(name);
        return nullptr;
    }
};

} 
#endif