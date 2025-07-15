#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"
#include <string>

class ConstantValueNode : public ExpressionNode {
  public:
    ConstantValueNode(const uint32_t line, const uint32_t col, int value, Type t);
    ConstantValueNode(const uint32_t line, const uint32_t col, double value, Type t);
    ConstantValueNode(const uint32_t line, const uint32_t col, const char* value, Type t);
    ConstantValueNode(const uint32_t line, const uint32_t col, bool value, Type t);
    ~ConstantValueNode() = default;

    char* getValue();
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    char* getType(){ return type; }

  private:
    int integer;
    double floating;
    bool boolean;
    const char* str;

    char type[10];
    char type_str[32];
};

#endif
