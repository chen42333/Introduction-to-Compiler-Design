#include "AST/function.hpp"

FunctionNode::FunctionNode(const uint32_t line, const uint32_t col, char* name, std::vector<DeclNode*> *decls, const char* constType, CompoundStatementNode* compound_stmt, Type t)
    : AstNode{line, col, t}, name(name), compound_stmt(compound_stmt), type(constType){
        if(decls != NULL) declarations = *decls;
        delete decls;
    }

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
     for(auto i: declarations) i->accept(p_visitor);
     if(compound_stmt != NULL) compound_stmt->accept(p_visitor);
}

char* FunctionNode::getName(){ return name; }

char* FunctionNode::getTypeString(){
    if(type != NULL) sprintf(type_str, "%s (", type);
    else sprintf(type_str, "void (");
    bool isStart = true;
    for(auto i: declarations){
        i->catVarType(isStart, type_str);
        isStart = false;
    }
    sprintf(type_str, "%s)", type_str);
    return type_str;
}