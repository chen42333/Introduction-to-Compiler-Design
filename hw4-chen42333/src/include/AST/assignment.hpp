#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include "AST/VariableReference.hpp"
#include "AST/expression.hpp"

class AssignmentNode : public AstNode {
  public:
    AssignmentNode(const uint32_t line, const uint32_t col, VariableReferenceNode* variable_ref, ExpressionNode* expr, Type t);
    ~AssignmentNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    VariableReferenceNode* getVarRef() { return variable_ref; }
    ExpressionNode* getExpr() { return expr; }

  private:
    VariableReferenceNode* variable_ref;
    ExpressionNode* expr;
};

#endif
