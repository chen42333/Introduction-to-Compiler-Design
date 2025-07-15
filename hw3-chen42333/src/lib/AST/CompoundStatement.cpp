#include "AST/CompoundStatement.hpp"
#include "AST/assignment.hpp"
#include "AST/for.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/while.hpp"
#include "AST/FunctionInvocation.hpp"

CompoundStatementNode::CompoundStatementNode(const uint32_t line, const uint32_t col, std::vector<DeclNode*> *decls, std::vector<AstNode*> *stmts, Type t)
    : AstNode{line, col, t}{
        if(decls != NULL) declarations = *decls;
        if(stmts != NULL) statements = *stmts;
        delete decls;
        delete stmts;
    }

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    for(auto i: declarations) i->accept(p_visitor);

    for(auto i: statements){
        switch(i->getType()){
            case compound_stmt:{
                CompoundStatementNode* tmp = (CompoundStatementNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case assignment:{
                AssignmentNode* tmp = (AssignmentNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case for_:{
                ForNode* tmp = (ForNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case if_:{
                IfNode* tmp = (IfNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case read:{
                ReadNode* tmp = (ReadNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case return_:{
                ReturnNode* tmp = (ReturnNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case while_:{
                WhileNode* tmp = (WhileNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case print_:{
                PrintNode* tmp = (PrintNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            case func_invoc:{
                FunctionInvocationNode* tmp = (FunctionInvocationNode*) i;
                tmp->accept(p_visitor);
                break;
            }
            default:
                break;
        }
    }
}