#include "AST/VariableReference.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"

VariableReferenceNode::VariableReferenceNode(const uint32_t line, const uint32_t col, char* name, std::vector<ExpressionNode*> *exprs, Type t)
    : ExpressionNode{line, col, t}, name(name){
        if(exprs != NULL) this->exprs = *exprs;
        delete exprs;
    }

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
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