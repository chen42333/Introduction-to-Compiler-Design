#include "AST/decl.hpp"
#include <string.h>

DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<char*> *IDList, std::vector<int> *line_list, std::vector<int> *col_list, const char* type, std::vector<int> *array_size_list, Type t)
    : AstNode{line, col, t} {
        VariableNode* tmp;
        for(int i = 0; i < IDList->size(); i++) {
            const int LINE = (*line_list)[i]; const int COL = (*col_list)[i];
            strcpy(this->type, type);
            tmp = new VariableNode(LINE, COL, (*IDList)[i], this->type, NULL, array_size_list, var);
            variables.push_back(tmp);
        }
        delete line_list;
        delete col_list;
        delete IDList;
    }
DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<char*> *IDList, std::vector<int> *line_list, std::vector<int> *col_list, ConstantValueNode* const_val, Type t)
    : AstNode{line, col, t} {
        VariableNode* tmp;
        for(int i = 0; i < IDList->size(); i++) {
            const int LINE = (*line_list)[i]; const int COL = (*col_list)[i];
            tmp = new VariableNode(LINE, COL, (*IDList)[i], const_val->getType(), const_val, NULL, var);
            variables.push_back(tmp);
        }
        delete line_list;
        delete col_list;
        delete IDList;
    }

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
     for(auto i: variables) i->accept(p_visitor);
}

void DeclNode::catVarType(bool isStart, char* cat){
    for(auto i: variables){
        if(isStart){
            sprintf(cat, "%s%s", cat, i->getType());
            isStart = false;
        }
        else sprintf(cat, "%s, %s", cat, i->getType());
    }
}
