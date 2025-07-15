#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/decl.hpp"

class FunctionNode : public AstNode {
  public:
    FunctionNode(const uint32_t line, const uint32_t col, char* name, std::vector<DeclNode*> *decls, const char* constType, CompoundStatementNode* compound_stmt, Type t);
    ~FunctionNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    char* getName();
    char* getType() { return (char*)type; }
    char* getReturnType();
    char* getTypeString();
    std::vector<DeclNode*>* getDecls() { return &declarations; }

  private:
    char* name;
    std::vector<DeclNode*> declarations;
    CompoundStatementNode* compound_stmt;
    const char* type;
    char return_type_str[256];
    char type_str[256];
};

#endif
