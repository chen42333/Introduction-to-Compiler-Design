#include "AST/while.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/VariableReference.hpp"
#include "AST/FunctionInvocation.hpp"

WhileNode::WhileNode(const uint32_t line, const uint32_t col, ExpressionNode* expr, CompoundStatementNode* compound_stmt, Type t)
    : AstNode{line, col, t}, expr(expr), compound_stmt(compound_stmt) {}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
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
    compound_stmt->accept(p_visitor);
}
