%{
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"
#include "AST/AstDumper.hpp"
#include "sema/SemanticAnalyzer.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%code requires {
    class AstNode;
    class CompoundStatementNode;
    class AssignmentNode;
    class BinaryOperatorNode;
    class ConstantValueNode;
    class DeclNode;
    class ExpressionNode;
    class ForNode;
    class FunctionNode;
    class FunctionInvocationNode;
    class IfNode;
    class PrintNode;
    class ProgramNode;
    class ReadNode;
    class ReturnNode;
    class UnaryOperatorNode;
    class VariableNode;
    class VariableReferenceNode;
    class WhileNode;
    #include <vector>
}

    /* For yylval */
%union {
    /* basic semantic value */
    char *identifier;
    int number;
    double floating;
    const char *string_;
    int isEpsilon;
    int isNeg;
    char *op;

    AstNode *node;
    CompoundStatementNode *compound_stmt_ptr;
    AssignmentNode *assignment_ptr;
    BinaryOperatorNode *binary_op_ptr;
    ConstantValueNode *const_val_ptr;
    DeclNode *decl_ptr;
    ExpressionNode *expr_ptr;
    ForNode *for_ptr;
    FunctionNode *func_ptr;
    FunctionInvocationNode *func_invoc_ptr;
    IfNode *if_ptr;
    PrintNode *print_ptr;
    ProgramNode *program_ptr;
    ReadNode *read_ptr;
    ReturnNode *return_ptr;
    UnaryOperatorNode *unary_op_ptr;
    VariableNode *var_ptr;
    VariableReferenceNode *var_ref_ptr;
    WhileNode *while_ptr;

    std::vector<int> *number_list;
    std::vector<char*> *identifier_list;
    std::vector<DeclNode*> *decl_list_ptr;
    std::vector<FunctionNode*> *func_list_ptr;
    std::vector<AstNode*> *stmt_list_ptr;
    std::vector<ExpressionNode*> *expr_list_ptr;

    struct {
        const char *string_;
        std::vector<int> *number_list;
    } type_cluster;

    struct {
        std::vector<char*> *identifier_list;
        std::vector<int> *id_line_list;
        std::vector<int> *id_column_list;
    } id_list_cluster;

    struct {
        AssignmentNode *assignment_ptr;
        PrintNode *print_ptr;
        ReadNode *read_ptr;
    } simple_stmt_cluster;
};

%type <node> Statement
%type <identifier> ProgramName FunctionName ID
%type <number> INT_LITERAL
%type <floating> REAL_LITERAL
%type <string_> ScalarType ReturnType BOOLEAN INTEGER REAL STRING STRING_LITERAL
%type <op> ASSIGN OR AND NOT LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL PLUS MINUS MULTIPLY DIVIDE MOD
%type <isNeg> NegOrNot
%type <program_ptr> Program
%type <compound_stmt_ptr> CompoundStatement ElseOrNot
%type <decl_ptr> Declaration FormalArg
%type <func_ptr> Function FunctionDeclaration FunctionDefinition
%type <func_invoc_ptr> FunctionCall FunctionInvocation
%type <const_val_ptr> LiteralConstant StringAndBoolean IntegerAndReal
%type <var_ref_ptr> VariableReference
%type <expr_ptr> Expression
%type <if_ptr> Condition
%type <while_ptr> While
%type <for_ptr> For
%type <return_ptr> Return
%type <number_list> ArrDecl
%type <decl_list_ptr> DeclarationList Declarations FormalArgList FormalArgs
%type <stmt_list_ptr> StatementList Statements
%type <func_list_ptr> Functions FunctionList
%type <expr_list_ptr> ArrRefList ArrRefs ExpressionList Expressions
%type <type_cluster> ArrType Type
%type <id_list_cluster> IdList
%type <simple_stmt_cluster> Simple

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%

ProgramUnit:
    Program {root = $1;}
    |
    Function {root = $1;}
;

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END {
        $$ = new ProgramNode(@1.first_line, @1.first_column, $1, $3, $4, $5, program);
        free($1);
    }
;

ProgramName:
    ID {$$ = $1;}
;

DeclarationList:
    Epsilon {$$ = NULL;}
    |
    Declarations {$$ = $1;}
;

Declarations:
    Declaration {
        $$ = new std::vector<DeclNode*>(0);
        $$->push_back($1);
    }
    |
    Declarations Declaration {
        $$ = $1;
        $$->push_back($2);
    }
;

FunctionList:
    Epsilon {$$ = NULL;}
    |
    Functions {$$ = $1;}
;

Functions:
    Function {
        $$ = new std::vector<FunctionNode*>(0);
        $$->push_back($1);
    }
    |
    Functions Function {
        $$ = $1;
        $$->push_back($2);
    }
;

Function:
    FunctionDeclaration {$$ = $1;}
    |
    FunctionDefinition {$$ = $1;}
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON {$$ = new FunctionNode(@1.first_line, @1.first_column, $1, $3, $5, NULL, func);}
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType 
    CompoundStatement
    END {$$ = new FunctionNode(@1.first_line, @1.first_column, $1, $3, $5, $6, func);}
;

FunctionName:
    ID {$$ = $1;}
;

FormalArgList:
    Epsilon {$$ = NULL;}
    |
    FormalArgs {$$ = $1;}
;

FormalArgs:
    FormalArg {
        $$ = new std::vector<DeclNode*>(0);
        $$->push_back($1);
    }
    |
    FormalArgs SEMICOLON FormalArg {
        $$ = $1;
        $$->push_back($3);
    }
;

FormalArg:
    IdList COLON Type {$$ = new DeclNode(@1.first_line, @1.first_column, $1.identifier_list, $1.id_line_list, $1.id_column_list, $3.string_, $3.number_list, decl);}
;

IdList:
    ID {
        $$.identifier_list = new std::vector<char*>(0);
        ($$.identifier_list)->push_back($1);
        $$.id_line_list = new std::vector<int>(0);
        ($$.id_line_list)->push_back(@1.first_line);
        $$.id_column_list = new std::vector<int>(0);
        ($$.id_column_list)->push_back(@1.first_column);
    }
    |
    IdList COMMA ID {
        $$.identifier_list = $1.identifier_list;
        ($$.identifier_list)->push_back($3);
        $$.id_line_list = $1.id_line_list;
        ($$.id_line_list)->push_back(@3.first_line);
        $$.id_column_list = $1.id_column_list;
        ($$.id_column_list)->push_back(@3.first_column);
    }
;

ReturnType:
    COLON ScalarType {$$ = $2;}
    |
    Epsilon {$$ = NULL;}
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2.identifier_list, $2.id_line_list, $2.id_column_list, $4.string_, $4.number_list, decl);
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2.identifier_list, $2.id_line_list, $2.id_column_list, $4, decl);
    }
;

Type:
    ScalarType {
        $$.string_ = $1;
        $$.number_list = NULL;
    }
    |
    ArrType {
        $$.string_ = $1.string_;
        $$.number_list = $1.number_list;
    }
;

ScalarType:
    INTEGER {
        $$ = $1;
    }
    |
    REAL {
        $$ = $1;
    }
    |
    STRING {
        $$ = $1;
    }
    |
    BOOLEAN {
        $$ = $1;
    }
;

ArrType:
    ArrDecl ScalarType {
        $$.string_ = $2;
        $$.number_list = $1;
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF {
        $$ = new std::vector<int>(0);
        $$->push_back($2);
    }
    |
    ArrDecl ARRAY INT_LITERAL OF {
        $$ = $1;
        $$->push_back($3);
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL {
        if($1) $$ = new ConstantValueNode(@1.first_line, @1.first_column, -($2), const_val);
        else $$ = new ConstantValueNode(@2.first_line, @2.first_column, $2, const_val);
    }
    |
    NegOrNot REAL_LITERAL {
        if($1) $$ = new ConstantValueNode(@1.first_line, @1.first_column, -($2), const_val);
        else $$ = new ConstantValueNode(@2.first_line, @2.first_column, $2, const_val);
    }
    |
    StringAndBoolean {$$ = $1;}
;

NegOrNot:
    Epsilon {$$ = 0;}
    |
    MINUS %prec UNARY_MINUS {$$ = 1;}
;

StringAndBoolean:
    STRING_LITERAL {$$ = new ConstantValueNode(@1.first_line, @1.first_column, $1, const_val);}
    |
    TRUE {$$ = new ConstantValueNode(@1.first_line, @1.first_column, true, const_val);}
    |
    FALSE {$$ = new ConstantValueNode(@1.first_line, @1.first_column, false, const_val);}
;

IntegerAndReal:
    INT_LITERAL {$$ = new ConstantValueNode(@1.first_line, @1.first_column, $1, const_val);}
    |
    REAL_LITERAL {$$ = new ConstantValueNode(@1.first_line, @1.first_column, $1, const_val);}
;

    /*
       Statements
                  */

Statement:
    CompoundStatement {$$ = $1;}
    |
    Simple {
        if($1.print_ptr != NULL) $$ = $1.print_ptr;
        else if($1.read_ptr != NULL) $$ = $1.read_ptr;
        else if($1.assignment_ptr != NULL) $$ = $1.assignment_ptr;
    }
    |
    Condition {$$ = $1;}
    |
    While {$$ = $1;}
    |
    For {$$ = $1;}
    |
    Return {$$ = $1;}
    |
    FunctionCall {$$ = $1;}
;

CompoundStatement:
    BEGIN_
    DeclarationList
    StatementList
    END {
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3, compound_stmt);
    }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON  {$$.assignment_ptr = new AssignmentNode(@2.first_line, @2.first_column, $1, $3, assignment);} 
    |
    PRINT Expression SEMICOLON {$$.print_ptr = new PrintNode(@1.first_line, @1.first_column, $2, print_);}
    |
    READ VariableReference SEMICOLON {$$.read_ptr = new ReadNode(@1.first_line, @1.first_column, $2, read);}
;

VariableReference:
    ID ArrRefList {$$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, $2, var_ref);}
;

ArrRefList:
    Epsilon {$$ = NULL;}
    |
    ArrRefs {$$ = $1;}
;

ArrRefs:
    L_BRACKET Expression R_BRACKET {
        $$ = new std::vector<ExpressionNode*>(0);
        $$->push_back($2);
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET {
        $$ = $1;
        $$->push_back($3);
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF {$$ = new IfNode(@1.first_line, @1.first_column, $2, $4, $5, if_);}
;

ElseOrNot:
    ELSE
    CompoundStatement {$$ = $2;}
    |
    Epsilon {$$ = NULL;}
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO {$$ = new WhileNode(@1.first_line, @1.first_column, $2, $4, while_);}
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO {
        ConstantValueNode* const_val_ptr_1 = new ConstantValueNode(@4.first_line, @4.first_column, $4, const_val);
        VariableReferenceNode* var_ref_ptr = new VariableReferenceNode(@2.first_line, @2.first_column, $2, NULL, var_ref);
        AssignmentNode* assignment_ptr = new AssignmentNode(@3.first_line, @3.first_column, var_ref_ptr, const_val_ptr_1, assignment);
        
        std::vector<char*>* identifier_list = new std::vector<char*>(0);
        identifier_list->push_back($2);
        std::vector<int>* id_line_list = new std::vector<int>(0);
        id_line_list->push_back(@2.first_line);
        std::vector<int>* id_column_list = new std::vector<int>(0);
        id_column_list->push_back(@2.first_column);
        const char* type = "integer";
        DeclNode* decl_ptr = new DeclNode(@2.first_line, @2.first_column, identifier_list, id_line_list, id_column_list, type, NULL, decl);

        ConstantValueNode* const_val_ptr_2 = new ConstantValueNode(@6.first_line, @6.first_column, $6, const_val);
        $$ = new ForNode(@1.first_line, @1.first_column, decl_ptr, assignment_ptr, const_val_ptr_2, $8, for_);
        }
;

Return:
    RETURN Expression SEMICOLON {$$ = new ReturnNode(@1.first_line, @1.first_column, $2, return_);}
;

FunctionCall:
    FunctionInvocation SEMICOLON {$$ = $1;}
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS {$$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, $3, func_invoc);}
;

ExpressionList:
    Epsilon {$$ = NULL;}
    |
    Expressions {$$ = $1;}
;

Expressions:
    Expression {
        $$ = new std::vector<ExpressionNode*>(0);
        $$->push_back($1);
    }
    |
    Expressions COMMA Expression {
        $$ = $1;
        $$->push_back($3);
    }
;

StatementList:
    Epsilon {$$ = NULL;}
    |
    Statements {$$ = $1;}
;

Statements:
    Statement {
        $$ = new std::vector<AstNode*>(0);
        $$->push_back($1);
    }
    |
    Statements Statement{
        $$ = $1;
        $$->push_back($2);
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS {$$ = $2; }
    |
    MINUS Expression %prec UNARY_MINUS {
        char neg[] = "neg";
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, strdup(neg), $2, unary_op);}
    |
    Expression MULTIPLY Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression DIVIDE Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression MOD Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression PLUS Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression MINUS Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression LESS Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression LESS_OR_EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression GREATER Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression GREATER_OR_EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression NOT_EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    NOT Expression {$$ = new UnaryOperatorNode(@1.first_line, @1.first_column, $1, $2, unary_op);}
    |
    Expression AND Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    Expression OR Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3, binary_op);}
    |
    IntegerAndReal {$$ = $1;}
    |
    StringAndBoolean {$$ = $1;}
    |
    VariableReference {$$ = $1; }
    |
    FunctionInvocation {$$ = $1;}
;

    /*
       misc
            */
Epsilon: 
;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        AstDumper ast_dumper;
        root->accept(ast_dumper);
    }

    SemanticAnalyzer sema_analyzer(argv[1]);
    root->accept(sema_analyzer);

    // TODO: do not print this if there's any semantic error
    if (sema_analyzer.correct()) {
        printf("\n"
            "|---------------------------------------------------|\n"
            "|  There is no syntactic error and semantic error!  |\n"
            "|---------------------------------------------------|\n");
    }

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}