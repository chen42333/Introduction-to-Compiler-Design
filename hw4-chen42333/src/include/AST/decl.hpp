#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ast.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/variable.hpp"
#include <vector>

class DeclNode : public AstNode {
  public:
    // variable declaration
    DeclNode(const uint32_t line, const uint32_t col, std::vector<char*> *IDList, std::vector<int> *line_list, std::vector<int> *col_list, const char* type, std::vector<int> *array_size_list, Type t);
    // constant variable declaration
    DeclNode(const uint32_t line, const uint32_t col, std::vector<char*> *IDList, std::vector<int> *line_list, std::vector<int> *col_list, ConstantValueNode* const_val, Type t);

    ~DeclNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    void catVarType(bool isStart, char* cat);
    std::vector<VariableNode*>* getVars() { return &variables; }

  private:
    std::vector<VariableNode*> variables;
    char type[8];
};

#endif
