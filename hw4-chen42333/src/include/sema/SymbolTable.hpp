#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <vector>
#include <stack>
#include <string>
#include "AST/ast.hpp"
#include "AST/decl.hpp"

enum Kind { prog, function, parameter, variable, loop_var, constant };

class SymbolEntry {
    public:
        SymbolEntry(): name(NULL) {}
        SymbolEntry(char *name, Kind kind, int level, char* type, char *attribute, int num_idx_arg, std::vector<DeclNode*>* decls): name(name), kind(kind), level(level), type(type), attribute(attribute), num_idx_arg(num_idx_arg), decls(decls){}
        int getLevel() { return level; }
        char* getName() { return name; }
        char* getKindStr();
        Kind getKind() { return kind;}
        char* getType() { return type; }
        char* getAttr() { return attribute; }
        void setError() { error = true; }
        bool getError() { return error; }
        int getNumIndecies() { return num_idx_arg; }
        std::vector<DeclNode*>* getDecls() { return decls; }
    private:
        // Variable names
        // Kind
        // Level
        // ...
        char* name;
        Kind kind;
        int level;
        char* type;
        char* attribute;
        char kind_str[15];
        bool error = false;
        int num_idx_arg;
        std::vector<DeclNode*>* decls;
};

class SymbolTable {
public:
    SymbolTable(int level, char* owner):level(level), owner(owner){}
    void addSymbol(char *name, Kind kind, int level, char* type, char *attribute, int num_idx_arg, std::vector<DeclNode*> *decls);
    int getLevel() { return level; }
    SymbolEntry find_var(char*);
    void clear() { entries.clear(); }
    std::vector<SymbolEntry>* getTable() { return &entries; };
    void setLastVarError() { entries.back().setError(); }
    char* getOwner() { return owner; }
    // other methods
private:
    int level;
    char* owner;
    std::vector<SymbolEntry> entries;
};

class SymbolManager {
public:
    void pushScope(SymbolTable *new_scope){ tables.push(new_scope); }
    void popScope();
    SymbolTable* currentTable(){ return tables.top(); }
    int currentLevel();
    SymbolEntry find_var(char*);
    char* currentOwner();
    // other methods
private:
    std::stack <SymbolTable *> tables;
};

#endif