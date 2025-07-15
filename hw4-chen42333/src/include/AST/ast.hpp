#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

#include <cstdint>
#include <vector>

enum Type {compound_stmt, assignment, binary_op, const_val, decl, expr, for_, func, func_invoc, if_, print_, program, read, return_, 
unary_op, var, var_ref, while_};

struct _formal_arg{
    const char* type;
    std::vector<int> *number_list;
    std::vector<char*> *identifier_list;
    std::vector<int> *id_line_list;
    std::vector<int> *id_column_list;
};

class AstNodeVisitor;

struct Location {
    uint32_t line;
    uint32_t col;

    ~Location() = default;
    Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}
};

class AstNode {
  protected:
    Location location;
    Type type;

  public:
    virtual ~AstNode() = 0;
    AstNode(const uint32_t line, const uint32_t col, Type t);

    AstNode(const AstNode &) = delete;
    AstNode(AstNode &&) = delete;
    AstNode &operator=(const AstNode &) = delete;
    AstNode &operator=(AstNode &&) = delete;
    
    const Location &getLocation() const;
    
    virtual void accept(AstNodeVisitor &p_visitor) = 0;
    virtual void visitChildNodes(AstNodeVisitor &p_visitor){};

    Type getType() { return type; }
};

#endif
