#include "AST/return.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/VariableReference.hpp"
#include "AST/FunctionInvocation.hpp"

ReturnNode::ReturnNode(const uint32_t line, const uint32_t col, ExpressionNode* expr, Type t)
    : AstNode{line, col, t}, expr(expr) {}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    switch(expr->getType()){
        case binary_op:{
            BinaryOperatorNode* tmp = (BinaryOperatorNode*) expr;
            tmp->accept(p_visitor);
            break;
        }
        case unary_op:{
            UnaryOperatorNode* tmp = (UnaryOperatorNode*) expr;
            tmp->accept(p_visitor);
            break;
        }
        case const_val:{
            ConstantValueNode* tmp = (ConstantValueNode*) expr;
            tmp->accept(p_visitor);
            break;
        }
        case var_ref:{
            VariableReferenceNode* tmp = (VariableReferenceNode*) expr;
            tmp->accept(p_visitor);
            break;
        }
        case func_invoc:{
            FunctionInvocationNode* tmp = (FunctionInvocationNode*) expr;
            tmp->accept(p_visitor);
            break;
        }
        default:
            break;
    }
}