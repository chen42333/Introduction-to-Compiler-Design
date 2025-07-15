#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/VariableReference.hpp"
#include "AST/FunctionInvocation.hpp"

BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col, char* op, ExpressionNode* left, ExpressionNode* right, Type t)
    : ExpressionNode{line, col, t}, op(op){
        exprs.push_back(left); exprs.push_back(right);
    }

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    for(auto expr: exprs){
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
}
