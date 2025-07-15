#ifndef __AST_IF_NODE_H
#define __AST_IF_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/CompoundStatement.hpp"

class IfNode : public AstNode {
  public:
    IfNode(const uint32_t line, const uint32_t col, ExpressionNode* expr, CompoundStatementNode* then_act, CompoundStatementNode* else_act,Type t);
    ~IfNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    ExpressionNode* getExpr() { return expr; }

  private:
    ExpressionNode* expr;
    std::vector<CompoundStatementNode*> acts;
};

#endif
