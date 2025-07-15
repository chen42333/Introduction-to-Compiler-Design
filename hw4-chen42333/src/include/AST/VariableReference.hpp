#ifndef __AST_VARIABLE_REFERENCE_NODE_H
#define __AST_VARIABLE_REFERENCE_NODE_H

#include "AST/expression.hpp"

class VariableReferenceNode : public ExpressionNode {
  public:
    // normal reference
    VariableReferenceNode(const uint32_t line, const uint32_t col, char* name, std::vector<ExpressionNode*> *exprs, Type t);
    // array reference
    // VariableReferenceNode(const uint32_t line, const uint32_t col, name, expressions, type);
    ~VariableReferenceNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    char* getName(){ return name; };
    std::vector<ExpressionNode*>* getIndecies () { return &exprs; }

  private:
    char* name;
    std::vector<ExpressionNode*> exprs;
};

#endif
