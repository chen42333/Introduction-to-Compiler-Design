#include "codegen/CodeGenerator.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>

CodeGenerator::CodeGenerator(const std::string &source_file_name,
                             const std::string &save_path,
                             const SymbolManager *const p_symbol_manager)
    : m_symbol_manager_ptr(p_symbol_manager),
      m_source_file_path(source_file_name) {
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path =
        save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind("/");
    auto dot_pos = source_file_name.rfind(".");

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    auto output_file_path{
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue,
                     m_source_file_path.c_str());

    // Reconstruct the hash table for looking up the symbol entry
    // Hint: Use symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_program.getSymbolTable());

    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclNodes().begin(), p_program.getDeclNodes().end(),
             visit_ast_node);
    in_func = true;
    for_each(p_program.getFuncNodes().begin(), p_program.getFuncNodes().end(),
             visit_ast_node);
    in_func = false;

    const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);


    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_program.getSymbolTable());
}

void CodeGenerator::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void CodeGenerator::visit(VariableNode &p_variable) {
    if (!in_func) {
        if (p_variable.getConstantPtr() == nullptr) { // variable
            constexpr const char *const global_var = ".comm %s, 4, 4\n";
            dumpInstructions(m_output_file.get(), global_var, p_variable.getNameCString());
        }
        else { // constant
            constexpr const char *const global_const = 
                ".section    .rodata\n"
                "    .align 2\n"
                "    .globl %s\n"
                "    .type %s, @object\n"
                "%s:\n"
                "    .word %s\n";
            dumpInstructions(m_output_file.get(), global_const, p_variable.getNameCString(), p_variable.getNameCString(), p_variable.getNameCString(), p_variable.getConstantPtr()->getConstantValueCString());
        }
    }
    else {
        SymbolEntry* symbol_entry = (SymbolEntry*) m_symbol_manager_ptr->lookup(p_variable.getName());
        var_addr += 4;
        symbol_entry->setAddr(var_addr);
        if (p_variable.getConstantPtr() != nullptr) { // constant
            constexpr const char *const local_var = 
            "    li t0, %s\n"
            "    addi t1, s0, -%d\n"
            "    sw t0, 0(t1)\n";
            dumpInstructions(m_output_file.get(), local_var, p_variable.getConstantPtr()->getConstantValueCString(), var_addr);
        }
    }
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    constexpr const char *const push = 
        "    li t0, %s\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)\n";
    dumpInstructions(m_output_file.get(), push, p_constant_value.getConstantValueCString());

    if (for_cond_setting) {
        constexpr const char *const loop_var_comp = 
            "    lw t0, 0(sp)\n"
            "    addi sp, sp, 4\n"
            "    lw t1, 0(sp)\n"
            "    addi sp, sp, 4\n"
            "    bge t1, t0, L%d\n"
            "L%d:\n";
        dumpInstructions(m_output_file.get(), loop_var_comp, tags.top()[1], tags.top()[0]);
        cur_tag = tags.top()[0];
    }
}

void CodeGenerator::visit(FunctionNode &p_function) {
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_function.getSymbolTable());

    constexpr const char *const func_start = 
            ".section    .text\n"
            "    .align 2\n"
            "    .globl %s\n"
            "    .type %s, @function\n"
            "%s:\n"
            "    addi sp, sp, -128\n"
            "    sw ra, 124(sp)\n"
            "    sw s0, 120(sp)\n"
            "    addi s0, sp, 128\n";
        dumpInstructions(m_output_file.get(), func_start, p_function.getNameCString(), p_function.getNameCString(), p_function.getNameCString());

    var_addr = 8;
    int i = 0;
    for (const auto& decl: p_function.getParameters()) {
        auto &vars = decl->getVariables();
        for (const auto& var: vars) {
            var_addr += 4;
            if (i < 8) {
                constexpr const char *const arg = "    sw a%d, -%d(s0)\n";
                dumpInstructions(m_output_file.get(), arg, i, var_addr);
            }
            SymbolEntry* symbol_entry = (SymbolEntry*) m_symbol_manager_ptr->lookup(var->getNameCString());
            symbol_entry->setAddr(var_addr);
            i++;
        }
    }
    p_function.visitBodyChildNodes(*this);

    constexpr const char *const func_end = 
        "    lw ra, 124(sp)\n"
        "    lw s0, 120(sp)\n"
        "    addi sp, sp, 128 \n"
        "    jr ra        \n"
        "    .size %s, .-%s\n";
    dumpInstructions(m_output_file.get(), func_end, p_function.getNameCString(), p_function.getNameCString());

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_function.getSymbolTable());
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_compound_statement.getSymbolTable());
    bool in_func_back = in_func;
    bool body_back = body;
    bool for_cond_setting_back = for_cond_setting;

    if (!in_func_back) {
        constexpr const char *const main_func = 
            ".section    .text\n"
            "    .align 2\n"
            "    .globl main\n"
            "    .type main, @function\n"
            "main:\n"
            "    addi sp, sp, -128\n"
            "    sw ra, 124(sp)\n"
            "    sw s0, 120(sp)\n"
            "    addi s0, sp, 128\n";
        dumpInstructions(m_output_file.get(), main_func);
        var_addr = 8;
        in_func = true;
    }

    for_cond_setting = false;
    p_compound_statement.visitChildNodes(*this);

    if (!in_func_back) {
        constexpr const char *const main_func = 
            "    lw ra, 124(sp)\n"
            "    lw s0, 120(sp)\n"
            "    addi sp, sp, 128 \n"
            "    jr ra        \n"
            "    .size main, .-main\n";
        dumpInstructions(m_output_file.get(), main_func);
    }

    if (for_cond_setting_back) {
        constexpr const char *const loop_var_increment = 
            "    addi t0, s0, -%d\n"
            "    lw t1, -%d(s0)\n"
            "    addi t1, t1, 1\n"
            "    sw t1, 0(t0)\n";
        dumpInstructions(m_output_file.get(), loop_var_increment, loop_var_addr.top(), loop_var_addr.top());
    }

    if (body_back) {
        if (tags.top()[1] > -1) { // if body or while/for body
            if (tags.top()[2] > -1) { // has else body
                constexpr const char *const jump = "    j L%d\n";
                dumpInstructions(m_output_file.get(), jump, tags.top()[2]);
            }
            else if (tags.top()[2] == -2) { // while/for body
                constexpr const char *const jump = "    j L%d\n";
                dumpInstructions(m_output_file.get(), jump, loop_cond_tag.top());
            }
            constexpr const char *const next_tag = "L%d:\n";
            dumpInstructions(m_output_file.get(), next_tag, tags.top()[1]);
            tags.top()[1] = -1; // mark that the if body has been compiled
            cur_tag = tags.top()[1];
        }
        else { // else body
            constexpr const char *const next_tag = "L%d:\n";
            dumpInstructions(m_output_file.get(), next_tag, tags.top()[2]);
            cur_tag = tags.top()[2];
        }
    }

    body = body_back;

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(
        p_compound_statement.getSymbolTable());
}

void CodeGenerator::visit(PrintNode &p_print) {
    p_print.visitChildNodes(*this);

    constexpr const char *const print_instr = 
    "    lw a0, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    jal ra, printInt\n";
    dumpInstructions(m_output_file.get(), print_instr);
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    p_bin_op.visitChildNodes(*this);

    char operation[100];
    switch (p_bin_op.getOp()) {
    case Operator::kPlusOp:
        sprintf(operation, "    add t0, t1, t0\n");
        break;
    case Operator::kMinusOp:
        sprintf(operation, "    sub t0, t1, t0\n");
        break;
    case Operator::kMultiplyOp:
        sprintf(operation, "    mul t0, t1, t0\n");
        break;
    case Operator::kDivideOp:
        sprintf(operation, "    div t0, t1, t0\n");
        break;
    case Operator::kModOp:
        sprintf(operation, "    rem t0, t1, t0\n");
        break;
    case Operator::kEqualOp:
        if (condition) sprintf(operation, "    bne t1, t0, L%d\nL%d:\n", tags.top()[1], tags.top()[0]);
        break;
    case Operator::kGreaterOp:
        if (condition) sprintf(operation, "    ble t1, t0, L%d\nL%d:\n", tags.top()[1], tags.top()[0]);
        break;
    case Operator::kGreaterOrEqualOp:
        if (condition) sprintf(operation, "    blt t1, t0, L%d\nL%d:\n", tags.top()[1], tags.top()[0]);
        break;
    case Operator::kLessOp:
        if (condition) sprintf(operation, "    bge t1, t0, L%d\nL%d:\n", tags.top()[1], tags.top()[0]);
        break;
    case Operator::kLessOrEqualOp:
        if (condition) sprintf(operation, "    bgt t1, t0, L%d\nL%d:\n", tags.top()[1], tags.top()[0]);
        break;
    case Operator::kNotEqualOp:
        if (condition) sprintf(operation, "    beq t1, t0, L%d\nL%d:\n", tags.top()[1], tags.top()[0]);
        break;
    default:
        break;
    }

    constexpr const char *const compute = 
        "    lw t0, 0(sp)\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)\n"
        "    addi sp, sp, 4\n"
        "%s";
    dumpInstructions(m_output_file.get(), compute, operation);

    if (condition) {
        condition = false;
        cur_tag = tags.top()[0];
    }
    else {
        constexpr const char *const push = 
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)\n";
        dumpInstructions(m_output_file.get(), push);
    }
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    p_un_op.visitChildNodes(*this);
    constexpr const char *const inverse = 
    "    lw t0, 0(sp)\n"
    "    sub t0, zero, t0\n"
    "    sw t0, 0(sp)\n";
    dumpInstructions(m_output_file.get(), inverse);

}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    p_func_invocation.visitChildNodes(*this);

    for (int i = p_func_invocation.getArguments().size()-1; i >= 0; i--) {
        if (i >= 8) {
            constexpr const char *const arg = 
                "    lw t0, 0(sp)\n"
                "    sw t0, -8(sp)\n"
                "    addi sp, sp, 4\n";
            dumpInstructions(m_output_file.get(), arg);
        }
        else {
            constexpr const char *const arg = 
                "    lw a%d, 0(sp)\n"
                "    addi sp, sp, 4\n";
            dumpInstructions(m_output_file.get(), arg, i);
        }
    }
    constexpr const char *const func = 
        "    jal ra, %s\n"
        "    mv t0, a0\n"
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp) \n";
    dumpInstructions(m_output_file.get(), func, p_func_invocation.getNameCString());
 }

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    // integer don't need to visit children
    SymbolEntry* symbol_entry = (SymbolEntry*)m_symbol_manager_ptr->lookup(p_variable_ref.getNameCString());
    if (var_lhs) {
        // push address
        if (symbol_entry->getAddr() == 0) {
            //global var
            constexpr const char *const push = "    la t0, %s\n";
            dumpInstructions(m_output_file.get(), push, symbol_entry->getNameCString());
        }
        else {
            //local var
            constexpr const char *const push = "    addi t0, s0, -%d\n";
            dumpInstructions(m_output_file.get(), push, symbol_entry->getAddr());
        }
    }
    else {
        // push value
        if (symbol_entry->getAddr() == 0) {
            //global var
            constexpr const char *const push = "    la t0, %s\n";
            dumpInstructions(m_output_file.get(), push, symbol_entry->getNameCString());
        }
        else {
            //local var
            constexpr const char *const push = "    addi t0, s0, -%d\n";
            dumpInstructions(m_output_file.get(), push, symbol_entry->getAddr());
        }
        constexpr const char *const push = 
            "    lw t1, 0(t0)\n"
            "    mv t0, t1\n";
        dumpInstructions(m_output_file.get(), push);
    }
    constexpr const char *const push = 
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)\n";
    dumpInstructions(m_output_file.get(), push);
    var_lhs = false;
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    var_lhs = true;
    bool for_cond_setting_back = for_cond_setting;
    for_cond_setting = false;
    p_assignment.visitChildNodes(*this);

    constexpr const char *const pop = 
        "    lw t0, 0(sp)\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)\n"
        "    addi sp, sp, 4\n"
        "    sw t0, 0(t1)\n";
    dumpInstructions(m_output_file.get(), pop);

    if (for_cond_setting_back) {
        SymbolEntry* symbol_entry = (SymbolEntry*)m_symbol_manager_ptr->lookup(p_assignment.getLvalue().getNameCString());
        loop_var_addr.push(symbol_entry->getAddr());
        constexpr const char *const loop_var_comp = 
            "L%d:\n"
            "    lw t0, -%d(s0)\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)\n";
        dumpInstructions(m_output_file.get(), loop_var_comp, loop_cond_tag.top(), loop_var_addr.top());
        cur_tag = loop_cond_tag.top();
        for_cond_setting = true;
    }
}

void CodeGenerator::visit(ReadNode &p_read) {
    var_lhs = true;
    p_read.visitChildNodes(*this);

    constexpr const char *const read_instr = 
    "    jal ra, readInt\n"
    "    lw t0, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    sw a0, 0(t0)\n";
    dumpInstructions(m_output_file.get(), read_instr);
}

void CodeGenerator::visit(IfNode &p_if) {
    condition = true;
    body = true;
    if (p_if.has_else_body()) {
        int* tmp = new int(3);
        tmp[0] = tag;
        tmp[1] = tag + 1;
        tmp[2] = tag + 2;
        tag += 3;
        tags.push(tmp);
    }
    else {
        int* tmp = new int(3);
        tmp[0] = tag;
        tmp[1] = tag + 1;
        tmp[2] = -1;
        tag += 2;
        tags.push(tmp);
    }
    p_if.visitChildNodes(*this);
    body = false;
    delete tags.top();
    tags.pop();
}

void CodeGenerator::visit(WhileNode &p_while) {
    constexpr const char *const while_cond = "L%d:\n";
    dumpInstructions(m_output_file.get(), while_cond, tag);
    cur_tag = tag;

    condition = true;
    body = true;
    int* tmp = new int(3);
    tmp[0] = tag + 1;
    tmp[1] = tag + 2;
    tmp[2] = -2;
    tag += 3;
    tags.push(tmp);
    loop_cond_tag.push(cur_tag);
    p_while.visitChildNodes(*this);
    body = false;
    delete tags.top();
    tags.pop();
    loop_cond_tag.pop();
}

void CodeGenerator::visit(ForNode &p_for) {
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_for.getSymbolTable());

    body = true;
    for_cond_setting = true;
    loop_cond_tag.push(tag);
    int* tmp = new int(3);
    tmp[0] = tag + 1;
    tmp[1] = tag + 2;
    tmp[2] = -2;
    tag += 3;
    tags.push(tmp);
    p_for.visitChildNodes(*this);
    body = false;
    delete tags.top();
    tags.pop();
    loop_cond_tag.pop();
    loop_var_addr.pop();

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_for.getSymbolTable());
}

void CodeGenerator::visit(ReturnNode &p_return) {
    p_return.visitChildNodes(*this);
    constexpr const char *const ret = 
        "    lw t0, 0(sp)\n"
        "    addi sp, sp, 4\n"
        "    mv a0, t0\n";
    dumpInstructions(m_output_file.get(), ret);
}