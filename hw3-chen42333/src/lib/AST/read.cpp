#include "AST/read.hpp"

ReadNode::ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode* variable_ref, Type t)
    : AstNode{line, col, t}, variable_ref(variable_ref){}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
     variable_ref->accept(p_visitor);
}
