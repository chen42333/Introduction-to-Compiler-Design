#include "AST/expression.hpp"

ExpressionNode::ExpressionNode(const uint32_t line, const uint32_t col, Type t)
    : AstNode{line, col, t} {}
