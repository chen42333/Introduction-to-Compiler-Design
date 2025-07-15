#include <AST/ast.hpp>

// prevent the linker from complaining
AstNode::~AstNode() {}

AstNode::AstNode(const uint32_t line, const uint32_t col, Type t)
    : location(line, col), type(t) {}

const Location &AstNode::getLocation() const { return location; }
