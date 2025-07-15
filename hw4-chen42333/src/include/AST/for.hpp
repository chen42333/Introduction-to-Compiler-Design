#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include "AST/decl.hpp"
#include "AST/assignment.hpp"
#include "AST/expression.hpp"
#include "AST/CompoundStatement.hpp"

class ForNode : public AstNode {
  public:
    ForNode(const uint32_t line, const uint32_t col, DeclNode* declration, AssignmentNode* assignment, ExpressionNode* expr, CompoundStatementNode* compound_stmt, Type t);
    ~ForNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    AssignmentNode* getAssign() { return assignment; }
    ExpressionNode* getExpr() { return expr; }

  private:
    DeclNode* declration;
    AssignmentNode* assignment;
    ExpressionNode* expr;
    CompoundStatementNode* compound_stmt;
};

#endif
