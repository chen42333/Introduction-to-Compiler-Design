#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "visitor/AstNodeVisitor.hpp"
#include "sema/SymbolTable.hpp"
#include "AST/expression.hpp"

enum ERROR { redecl, zero_array, undeclared, non_var, non_int_idx, subscript, bin_type, ury_type, non_func, num_args, type_args, p_expr_scalar, r_var_scalar, r_const_loop_var, array_assign, const_assign, loop_var_assign, assign_array, assign_diff, cond_bool, loop_var_incre, return_void, return_type };

struct error_info {
  uint32_t line;
  uint32_t col;
  char* var;
  ERROR error_type;
  char* type_1;
  char* type_2;
};

extern uint32_t opt_table; /* declared in scanner.l */

class SemanticAnalyzer final : public AstNodeVisitor {
  private:
    // TODO: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    SymbolManager symbol_manager;
    bool inFor = false;
    std::vector<std::string> loop_var_name;
    bool enterFunc = false;
    bool newTable = true;
    SymbolEntry *loop_variable = NULL;
    Kind default_kind = variable;
    char* current_line;
    bool dump_table = true;
    char source_code[200][513];
    std::vector<error_info> error_msg;
    char* expr_type(ExpressionNode*);
    bool loop_var_redecl = false;
  public:
    ~SemanticAnalyzer();
    SemanticAnalyzer() = default;
    SemanticAnalyzer(const char*);
    
    bool correct() { return error_msg.size() == 0; }
    void setDumpTable(int value) { dump_table = (bool)value; }
    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;
};

#endif
