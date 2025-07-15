#include "sema/SymbolTable.hpp"
#include <string.h>
#include <iostream>

char* SymbolEntry::getKindStr() {
    switch (kind) {
    case prog:
        sprintf(kind_str, "program");
        break;
    case function:
        sprintf(kind_str, "function");
        break;
    case parameter:
        sprintf(kind_str, "parameter");
        break;
    case loop_var:
        sprintf(kind_str, "loop_var");
        break;
    case constant:
        sprintf(kind_str, "constant");
        break;
    case variable:
        sprintf(kind_str, "variable");
        break;
    default:
        break;
    }
    return kind_str;
}

void SymbolTable::addSymbol(char* name, Kind kind, int level, char* type, char* attribute, int num_idx_arg, std::vector<DeclNode*>* decls){
    SymbolEntry entry(name, kind, level, type, attribute, num_idx_arg, decls);
    entries.push_back(entry);
}

SymbolEntry SymbolTable::find_var(char* var){
    std::string var_name(var);
    for (int i = 0; i < entries.size(); i++) {
        if (strcmp(var, entries[i].getName()) == 0) return entries[i];
    }
    SymbolEntry tmp;
    return tmp;
}

void SymbolManager::popScope(){
    tables.top()->clear();
    delete tables.top();
    tables.pop();
}

int SymbolManager::currentLevel() { 
    if (tables.empty()) return 0;
    return tables.top()->getLevel(); 
}

SymbolEntry SymbolManager::find_var(char* var) {
    std::stack<SymbolTable*> buffer;
    SymbolEntry tar;
    while (!tables.empty() && tar.getName() == NULL) {
        buffer.push(tables.top());
        tables.pop();
        tar = buffer.top()->find_var(var);
    }
    while (!buffer.empty()) {
        tables.push(buffer.top());
        buffer.pop();
    }
    return tar;
}

char* SymbolManager::currentOwner() {
    std::stack<SymbolTable*> buffer;
    char* owner = NULL;
    while (!tables.empty() && owner == NULL) {
        buffer.push(tables.top());
        tables.pop();
        owner = buffer.top()->getOwner();
    }
    while (!buffer.empty()) {
        tables.push(buffer.top());
        buffer.pop();
    }
    return owner;
}