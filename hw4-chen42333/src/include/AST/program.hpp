#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/ast.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/decl.hpp"
#include "AST/function.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <memory>
#include <string>


class ProgramNode final : public AstNode {
  public:
    ~ProgramNode() = default;
    ProgramNode(const uint32_t line, const uint32_t col,
            const char *const p_name, std::vector<DeclNode*> *decls, std::vector<FunctionNode*> *funcs, CompoundStatementNode *const p_body, Type t);

    const char *getNameCString() const { return name.c_str(); }
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    
  private:
    std::string name;
    CompoundStatementNode *m_body;
    std::vector<DeclNode*> declarations;
    std::vector<FunctionNode*> functions;
};

#endif
