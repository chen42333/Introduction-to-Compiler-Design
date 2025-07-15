#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"
#include "AST/ConstantValue.hpp"

class VariableNode : public AstNode {
  public:
    VariableNode(const uint32_t line, const uint32_t col, char* name, char* type, ConstantValueNode* const_val, std::vector<int> *array_size_list, Type t);
    ~VariableNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    char* getType();
    char* getName();
    char* getValue();
    std::vector<int>* getArraySize() { return &array_size_list; }

    bool isConst() { return const_val != NULL; }

  private:
    char* name;
    char* type;
    ConstantValueNode* const_val;
    char type_str[256];
    std::vector<int> array_size_list;
};

#endif
