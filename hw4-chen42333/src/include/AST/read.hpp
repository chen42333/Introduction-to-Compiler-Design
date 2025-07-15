#ifndef __AST_READ_NODE_H
#define __AST_READ_NODE_H

#include "AST/ast.hpp"
#include "AST/VariableReference.hpp"

class ReadNode : public AstNode {
  public:
    ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode* variable_ref, Type t);
    ~ReadNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    VariableReferenceNode* getVarRef() { return variable_ref; }

  private:
    VariableReferenceNode* variable_ref;
};

#endif
