#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class BinaryOperatorNode : public ExpressionNode {
  public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col, char* op, ExpressionNode* left, ExpressionNode* right, Type t);
    ~BinaryOperatorNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    char* getOperator(){return op;}
    ExpressionNode* getLeft() { return exprs[0]; }
    ExpressionNode* getRight() { return exprs[1]; }

  private:
    char* op;
    std::vector<ExpressionNode*> exprs;
};

#endif
