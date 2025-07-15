#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"
#include <string.h>
#include <fstream>
#include <algorithm>
#include <iostream>
 #include <cstdio>

SemanticAnalyzer::SemanticAnalyzer(const char* file) {
    std::ifstream test(file);
    for (int i = 0; !test.eof(); i++) {
        test.getline(source_code[i], 512);
    }
    test.close();   
}

SemanticAnalyzer::~SemanticAnalyzer() {
    for (auto err: error_msg) {
        std::fprintf(stderr, "<Error> Found in line %u, column %u: ", err.line, err.col);
        switch (err.error_type) {
        case redecl:
            std::fprintf(stderr, "symbol '%s' is redeclared\n", err.var);
            break;
        case zero_array:
            std::fprintf(stderr, "'%s' declared as an array with an index that is not greater than 0\n", err.var);
            break;
        case undeclared:
            std::fprintf(stderr, "use of undeclared symbol '%s'\n", err.var);
            break;
        case non_var:
            std::fprintf(stderr, "use of non-variable symbol '%s'\n", err.var);
            break;
        case non_int_idx:
            std::fprintf(stderr, "index of array reference must be an integer\n");
            break;
        case subscript:
            std::fprintf(stderr, "there is an over array subscript on '%s'\n", err.var);
            break;
        case bin_type:
            std::fprintf(stderr, "invalid operands to binary operator '%s' ('%s' and '%s')\n", err.var, err.type_1, err.type_2);
            break;
        case ury_type:
            std::fprintf(stderr, "invalid operand to unary operator '%s' ('%s')\n", err.var, err.type_1);
            break;
        case non_func:
            std::fprintf(stderr, "call of non-function symbol '%s'\n", err.var);
            break;
        case num_args:
            std::fprintf(stderr, "too few/much arguments provided for function '%s'\n", err.var);
            break;
        case type_args: 
            std::fprintf(stderr, "incompatible type passing '%s' to parameter of type '%s'\n", err.type_1, err.type_2);
            break;
        case p_expr_scalar: 
            std::fprintf(stderr, "expression of print statement must be scalar type\n");
            break;
        case r_var_scalar: 
            std::fprintf(stderr, "variable reference of read statement must be scalar type\n");
            break;
        case r_const_loop_var: 
            std::fprintf(stderr, "variable reference of read statement cannot be a constant or loop variable\n");
            break;
        case array_assign: 
            std::fprintf(stderr, "array assignment is not allowed\n");
            break;
        case const_assign: 
            std::fprintf(stderr, "cannot assign to variable '%s' which is a constant\n", err.var);
            break;
        case loop_var_assign:
            std::fprintf(stderr, "the value of loop variable cannot be modified inside the loop body\n");
            break;
        case assign_array:
            std::fprintf(stderr, "array assignment is not allowed\n");
            break;
        case assign_diff:
            std::fprintf(stderr, "assigning to '%s' from incompatible type '%s'\n", err.type_1, err.type_2);
            break;
        case cond_bool:
            std::fprintf(stderr, "the expression of condition must be boolean type\n");
            break;
        case loop_var_incre:
            std::fprintf(stderr, "the lower bound and upper bound of iteration count must be in the incremental order\n");
            break;
        case return_void:
            std::fprintf(stderr, "program/procedure should not return a value\n");
            break;
        case return_type:
            std::fprintf(stderr, "return '%s' from a function with return type '%s'\n", err.type_1, err.type_2);
            break;
        default:
            break;
        }
        std::fprintf(stderr, "    %s\n    ", source_code[err.line-1]);
        for (int i = 0; i < err.col-1; i++) fprintf(stderr, " ");
        fprintf(stderr, "^\n");
    }
    error_msg.clear();
}

void dumpDemarcation(const char chr) {
  for (size_t i = 0; i < 110; ++i) {
    printf("%c", chr);
  }
  puts("");
}

void dumpSymbol(SymbolTable* table) {
  dumpDemarcation('=');
  printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                                        "Attribute");
  dumpDemarcation('-');
  for (auto entry: *(table->getTable())) {
    printf("%-33s", entry.getName());
    printf("%-11s", entry.getKindStr());
    printf("%d", entry.getLevel());
    if (entry.getLevel() == 0) printf("%-10s", "(global)");
    else printf("%-10s", "(local)");
    printf("%-17s", entry.getType());
    printf("%-11s", entry.getAttr());
    puts("");
  }
  dumpDemarcation('-');
}

char* SemanticAnalyzer::expr_type(ExpressionNode* expr) {
    switch(expr->getType()){
        case binary_op:{
            BinaryOperatorNode* tmp = (BinaryOperatorNode*) expr;
            char* type_left = expr_type(tmp->getLeft());
            char* type_right = expr_type(tmp->getRight());
            char* type = new char(100);
            if (!strcmp("error", type_left) || !strcmp("error", type_right)) sprintf(type, "error");
            else if (!strcmp("+", tmp->getOperator())) {
                if (((!strcmp("integer", type_left)) || (!strcmp("real", type_left))) && 
                    ((!strcmp("integer", type_right)) || (!strcmp("real", type_right)))) sprintf(type, "real");
                else if (!strcmp("string", type_left) && !strcmp("string", type_right)) sprintf(type, "string");
                else sprintf(type, "error");
            }
            else if (!strcmp("-", tmp->getOperator()) || !strcmp("*", tmp->getOperator()) || !strcmp("/", tmp->getOperator())) {
                if (((!strcmp("integer", type_left)) || (!strcmp("real", type_left))) && 
                    ((!strcmp("integer", type_right)) || (!strcmp("real", type_right)))) sprintf(type, "real");
                else sprintf(type, "error");
            }
            else if (!strcmp("mod", tmp->getOperator())) {
                if (!strcmp("integer", type_left) && !strcmp("integer", type_right)) sprintf(type, "integer");
                else sprintf(type, "error");
            }
            else if (!strcmp("and", tmp->getOperator()) || !strcmp("or", tmp->getOperator())) {
                if (!strcmp("boolean", type_left) && !strcmp("boolean", type_right)) sprintf(type, "boolean");
                else sprintf(type, "error");
            }
            else if (!strcmp("<", tmp->getOperator()) || !strcmp("<=", tmp->getOperator()) || !strcmp("=", tmp->getOperator()) || !strcmp(">=", tmp->getOperator()) || !strcmp(">", tmp->getOperator()) || !strcmp("<>", tmp->getOperator())) {
                if (((!strcmp("integer", type_left)) || (!strcmp("real", type_left))) && 
                    ((!strcmp("integer", type_right)) || (!strcmp("real", type_right)))) sprintf(type, "boolean");
                else sprintf(type, "error");
            }
            else sprintf(type, "error");
            return type;
        }
        case unary_op:{
            char* type = expr_type(((UnaryOperatorNode*)expr)->getExpr());
            if (strcmp(type, "integer") && strcmp(type, "real") && strcmp(type, "boolean")) sprintf(type, "error");
            return type;
        }
        case const_val:
            return ((ConstantValueNode*)expr)->getType();
        case var_ref:{
            VariableReferenceNode* tmp = (VariableReferenceNode*) expr;
            SymbolEntry entry = symbol_manager.find_var(tmp->getName());
            if (entry.getError()|| entry.getName() == NULL || (entry.getKind() != parameter && entry.getKind() != variable && entry.getKind() != loop_var && entry.getKind() != constant)) {
                char* error = new char(6);
                sprintf(error, "error");
                return error;
            }
            char* type = new char(200);
            strcpy(type, entry.getType());
            for (int i = 0; i < tmp->getIndecies()->size(); i++) {
                char scalar_type[15], tail[100];
                memset(tail, 0, 100);
                int rm;
                sscanf(type, "%s [%d]%s", scalar_type, &rm, tail);
                strcpy(type, scalar_type);
                if (strlen(tail) > 0) {
                    strcat(type, " ");
                    strcat(type, tail);
                }
            }
            return type;
        }
        case func_invoc:{
            FunctionInvocationNode* tmp = (FunctionInvocationNode*) expr;
            SymbolEntry entry = symbol_manager.find_var(tmp->getName());
            if (entry.getError() || entry.getName() == NULL || entry.getKind() != function || entry.getNumIndecies() < tmp->getArgs()->size()) {
                char* error = new char(6);
                sprintf(error, "error");
                return error;
            }
            int idx_decl = 0, idx_var = 0;
            for (auto i: *(tmp->getArgs())) {
                char* type_tar = (*(*entry.getDecls())[idx_decl]->getVars())[idx_var++]->getType();
                char* type = expr_type(i);
                if (strcmp(type, type_tar) != 0) {
                    char* error = new char(6);
                    sprintf(error, "error");
                    return error;
                }
                if (idx_var == (*entry.getDecls())[idx_decl]->getVars()->size()) {
                    idx_var = 0;
                    idx_decl++;
                }
            }
            return entry.getType();
        }
        default:
            char* error = new char(6);
            sprintf(error, "error");
            return error;
    }
}

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    if (opt_table == 0) dump_table = false;
    // 1.
    SymbolTable *table = new SymbolTable(0, (char*)p_program.getNameCString());
    symbol_manager.pushScope(table);
    // 2.
    char* attr = "";
    table->addSymbol((char*)p_program.getNameCString(), prog, 0, "void", attr, 0, NULL);
    // 3.
    p_program.visitChildNodes(*this);
    // 5.
    if (dump_table) dumpSymbol(symbol_manager.currentTable());
    symbol_manager.popScope();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 2.
    if (default_kind == variable && p_variable.isConst()) default_kind = constant;
    else if (default_kind == constant && !p_variable.isConst()) default_kind = variable;
    char* attr = "";
    if (default_kind == constant) attr = p_variable.getValue();
    if (symbol_manager.currentTable()->find_var(p_variable.getName()).getName() != NULL || 
    (inFor && std::find(loop_var_name.begin(), loop_var_name.end(), std::string(p_variable.getName())) != loop_var_name.end())) {
        error_info tmp {p_variable.getLocation().line, p_variable.getLocation().col, p_variable.getName(), redecl};
        error_msg.push_back(tmp);
        return;
    }
    symbol_manager.currentTable()->addSymbol(p_variable.getName(), default_kind, symbol_manager.currentLevel(), p_variable.getType(), attr, p_variable.getArraySize()->size(), NULL);
    if(default_kind == loop_var) {
        std::string tmp = p_variable.getName();
        loop_var_name.push_back(tmp);
        default_kind = variable;
    }
    // 3.
    p_variable.visitChildNodes(*this);
    // 4.
    for (int i: *(p_variable.getArraySize())) {
        if (i <= 0) {
            error_info tmp {p_variable.getLocation().line, p_variable.getLocation().col, p_variable.getName(), zero_array};
            error_msg.push_back(tmp);
            symbol_manager.currentTable()->setLastVarError();
        }
    }
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 2.
    if (symbol_manager.currentTable()->find_var(p_function.getName()).getName() != NULL) {
        error_info tmp {p_function.getLocation().line, p_function.getLocation().col, p_function.getName(), redecl};
        error_msg.push_back(tmp);
    }
    else symbol_manager.currentTable()->addSymbol(p_function.getName(), function, symbol_manager.currentLevel(), p_function.getType(), p_function.getReturnType(), p_function.getDecls()->size(), p_function.getDecls());
    // 1.
    SymbolTable *table = new SymbolTable(symbol_manager.currentLevel()+1, p_function.getName());
    symbol_manager.pushScope(table);
    // 3.
    enterFunc = true;
    default_kind = parameter;
    p_function.visitChildNodes(*this);
    enterFunc = false;
    newTable = true;
    default_kind = variable;
    // 5.
    if (dump_table) dumpSymbol(symbol_manager.currentTable());
    symbol_manager.popScope();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 1.
    if (enterFunc && newTable) {
        enterFunc = false;
        newTable = false;
    }
    else if (!enterFunc) newTable = true;
    bool newTable_bk = newTable;
    if (newTable) {
        SymbolTable *table = new SymbolTable(symbol_manager.currentLevel()+1, symbol_manager.currentOwner());
        symbol_manager.pushScope(table);
    }
    // 3.
    if (default_kind == parameter) default_kind = variable;
    p_compound_statement.visitChildNodes(*this);
    // 5.
    if (newTable_bk) {
        if (dump_table) dumpSymbol(symbol_manager.currentTable());
        symbol_manager.popScope();
    }
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_print.visitChildNodes(*this);
    // 4.
    char* scalar_type[10], tail[100] = "";
    char* var_type = expr_type(p_print.getExpr());
    if (!strcmp("error", var_type)) return;
    sscanf(var_type, "%s %s", scalar_type, tail);
    if (strlen(tail) > 0 || !strcmp("void", var_type)) {
        error_info tmp {p_print.getExpr()->getLocation().line, p_print.getExpr()->getLocation().col, NULL, p_expr_scalar};
        error_msg.push_back(tmp);
    }
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_bin_op.visitChildNodes(*this);
    // 4.
    char* type = expr_type(&p_bin_op);
    char* type_left = expr_type(p_bin_op.getLeft());
    char* type_right = expr_type(p_bin_op.getRight());
    if (!strcmp("error", type) && strcmp("error", type_left) && strcmp("error", type_right)) {
        error_info tmp {p_bin_op.getLocation().line, p_bin_op.getLocation().col, p_bin_op.getOperator(), bin_type, type_left, type_right};
        error_msg.push_back(tmp);
    }
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_un_op.visitChildNodes(*this);
    // 4.
    char* type = expr_type(&p_un_op);
    char* type_expr = expr_type(p_un_op.getExpr());
    if (!strcmp("error", type) && strcmp("error", type_expr)) {
        error_info tmp {p_un_op.getLocation().line, p_un_op.getLocation().col, p_un_op.getOperator(), ury_type, type_expr};
        error_msg.push_back(tmp);
    }
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_func_invocation.visitChildNodes(*this);
    // 4.
    SymbolEntry entry = symbol_manager.find_var(p_func_invocation.getName());
    if (entry.getError()) return;
    if (entry.getName() == NULL) {
        error_info tmp {p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, p_func_invocation.getName(), undeclared};
        error_msg.push_back(tmp);
        return;
    }
    else if (entry.getKind() != function) {
        error_info tmp {p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, p_func_invocation.getName(), non_func};
        error_msg.push_back(tmp);
        return;
    }
    if (entry.getNumIndecies() < p_func_invocation.getArgs()->size()) {
        error_info tmp {p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, p_func_invocation.getName(), num_args};
        error_msg.push_back(tmp);
        return;
    }
    int idx_decl = 0, idx_var = 0;
    for (auto i: *(p_func_invocation.getArgs())) {
        char* type_tar = (*(*entry.getDecls())[idx_decl]->getVars())[idx_var++]->getType();
        char* type = expr_type(i);
        if (strcmp(type, type_tar) != 0) {
            error_info tmp {i->getLocation().line, i->getLocation().col, "NULL", type_args, type, type_tar};
            error_msg.push_back(tmp);
            return;
        }
        if (idx_var == (*entry.getDecls())[idx_decl]->getVars()->size()) {
            idx_var = 0;
            idx_decl++;
        }
    }
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_variable_ref.visitChildNodes(*this);
    // 4.
    SymbolEntry entry = symbol_manager.find_var(p_variable_ref.getName());
    if (entry.getError()) return;
    if (entry.getName() == NULL) {
        error_info tmp {p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, p_variable_ref.getName(), undeclared};
        error_msg.push_back(tmp);
        return;
    }
    else if (entry.getKind() != parameter && entry.getKind() != variable && entry.getKind() != loop_var && entry.getKind() != constant) {
        error_info tmp {p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, p_variable_ref.getName(), non_var};
        error_msg.push_back(tmp);
        return;
    }
    if (entry.getNumIndecies() < p_variable_ref.getIndecies()->size()) {
        error_info tmp {p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, p_variable_ref.getName(), subscript};
        error_msg.push_back(tmp);
        return;
    }
    for (auto i: *(p_variable_ref.getIndecies())) {
        char* idx_type = expr_type(i);
        if (!strcmp("error", idx_type)) return;
        if (strcmp("integer", idx_type) != 0) {
            error_info tmp {i->getLocation().line, i->getLocation().col, "NULL", non_int_idx};
            error_msg.push_back(tmp);
            return;
        }
    }
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_assignment.visitChildNodes(*this);
    // 4.
    char* scalar_type[10], tail[100] = "";
    char* var_type = expr_type(p_assignment.getVarRef());
    if (!strcmp("error", var_type)) return;
    sscanf(var_type, "%s %s", scalar_type, tail);
    if (strlen(tail) > 0) {
        error_info tmp {p_assignment.getVarRef()->getLocation().line, p_assignment.getVarRef()->getLocation().col, NULL, array_assign};
        error_msg.push_back(tmp);
        return;
    }
    Kind kind = symbol_manager.find_var(p_assignment.getVarRef()->getName()).getKind();
    if (kind == constant) {
        error_info tmp {p_assignment.getVarRef()->getLocation().line, p_assignment.getVarRef()->getLocation().col, p_assignment.getVarRef()->getName(), const_assign};
        error_msg.push_back(tmp);
        return;
    }
    else if (kind == loop_var && symbol_manager.currentTable()->find_var(p_assignment.getVarRef()->getName()).getName() == NULL) {
        if (default_kind == loop_var) default_kind = variable;
        else {
            error_info tmp {p_assignment.getVarRef()->getLocation().line, p_assignment.getVarRef()->getLocation().col, NULL, loop_var_assign};
            error_msg.push_back(tmp);
        }
        return;
    }
    sprintf(tail, "");
    char* var_type_2 = expr_type(p_assignment.getExpr());
    if (!strcmp("error", var_type_2)) return;
    sscanf(var_type_2, "%s %s", scalar_type, tail);
    if (strlen(tail) > 0) {
        error_info tmp {p_assignment.getExpr()->getLocation().line, p_assignment.getExpr()->getLocation().col, NULL, assign_array};
        error_msg.push_back(tmp);
        return;
    }
    if (strcmp(var_type, var_type_2) && (strcmp("integer", var_type) || strcmp("real", var_type_2)) && (strcmp("real", var_type) || strcmp("integer", var_type_2))) {
        error_info tmp {p_assignment.getLocation().line, p_assignment.getLocation().col, NULL, assign_diff, var_type, var_type_2};
        error_msg.push_back(tmp);
        return;
    }
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_read.visitChildNodes(*this);
    // 4.
    char* scalar_type[10], tail[100] = "";
    char* var_type = expr_type(p_read.getVarRef());
    if (!strcmp("error", var_type)) return;
    sscanf(var_type, "%s %s", scalar_type, tail);
    if (strlen(tail) > 0) {
        error_info tmp {p_read.getVarRef()->getLocation().line, p_read.getVarRef()->getLocation().col, NULL, r_var_scalar};
        error_msg.push_back(tmp);
        return;
    }
    Kind kind = symbol_manager.find_var(p_read.getVarRef()->getName()).getKind();
    if (kind == constant || kind == loop_var) {
        error_info tmp {p_read.getVarRef()->getLocation().line, p_read.getVarRef()->getLocation().col, NULL, r_const_loop_var};
        error_msg.push_back(tmp);
        return;
    }
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_if.visitChildNodes(*this);
    // 4.
    char* type = expr_type(p_if.getExpr());
    if (!strcmp("error", type)) return;
    else if (strcmp("boolean", type)) {
        error_info tmp {p_if.getExpr()->getLocation().line, p_if.getExpr()->getLocation().col, NULL, cond_bool};
        error_msg.push_back(tmp);
        return;
    }
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3.
    p_while.visitChildNodes(*this);
    // 4.
    char* type = expr_type(p_while.getExpr());
    if (!strcmp("error", type)) return;
    else if (strcmp("boolean", type)) {
        error_info tmp {p_while.getExpr()->getLocation().line, p_while.getExpr()->getLocation().col, NULL, cond_bool};
        error_msg.push_back(tmp);
        return;
    }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 1.
    SymbolTable *table = new SymbolTable(symbol_manager.currentLevel()+1, symbol_manager.currentOwner());
    symbol_manager.pushScope(table);
    // 3.
    inFor = true;
    default_kind = loop_var;
    int num_loop_var = loop_var_name.size();
    p_for.visitChildNodes(*this);
    inFor = false;
    loop_variable = NULL;
    if (loop_var_name.size() > num_loop_var) loop_var_name.pop_back(); //pop only if the loop variable is inserted successfully
    // 4.
    int lower = (atoi)(((ConstantValueNode*)p_for.getAssign()->getExpr())->getValue());
    int upper = (atoi)(((ConstantValueNode*)p_for.getExpr())->getValue());
    if (lower > upper) {
        error_info tmp {p_for.getLocation().line, p_for.getLocation().col, NULL, loop_var_incre};
        error_msg.push_back(tmp);
    }
    // 5.
    if (dump_table) dumpSymbol(symbol_manager.currentTable());
    symbol_manager.popScope();
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // 3. 
    p_return.visitChildNodes(*this);
    // 4.
    char* owner = symbol_manager.currentTable()->getOwner();
    char* tar_type = symbol_manager.find_var(owner).getType();
    if (!strcmp("void", tar_type)) {
        error_info tmp {p_return.getLocation().line, p_return.getLocation().col, NULL, return_void};
        error_msg.push_back(tmp);
        return;
    }
    char* ret_type = expr_type(p_return.getExpr());
    if (strcmp(ret_type, tar_type)) {
        error_info tmp {p_return.getExpr()->getLocation().line, p_return.getExpr()->getLocation().col, NULL, return_type, ret_type, tar_type};
        error_msg.push_back(tmp);
        return;
    }
}