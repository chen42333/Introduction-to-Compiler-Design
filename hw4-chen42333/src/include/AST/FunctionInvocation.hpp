#ifndef __AST_FUNCTION_INVOCATION_NODE_H
#define __AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"

class FunctionInvocationNode : public ExpressionNode{
  public:
    FunctionInvocationNode(const uint32_t line, const uint32_t col, char* name, std::vector<ExpressionNode*> *exprs, Type t);
    ~FunctionInvocationNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    char* getName(){ return name; };
    std::vector<ExpressionNode*>* getArgs() { return &exprs; }

  private:
    char* name;
    std::vector<ExpressionNode*> exprs;
};

#endif
