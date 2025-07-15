#include "AST/program.hpp"


ProgramNode::ProgramNode(const uint32_t line, const uint32_t col,
                         const char *const p_name, std::vector<DeclNode*> *decls, std::vector<FunctionNode*> *funcs, CompoundStatementNode *const p_body, Type t)
    : AstNode{line, col, t}, name(p_name), m_body(p_body){
        if(decls != NULL) declarations = *decls;
        if(funcs != NULL) functions = *funcs;
        delete decls;
        delete funcs;
    }

void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    for(auto i: declarations) i->accept(p_visitor);
    for(auto i: functions) i->accept(p_visitor);
     m_body->accept(p_visitor);
}