%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%start Program
%token COMMA SEMICOLON COLON L_PARENTHESIS R_PARENTHESIS L_BRACKET R_BRACKET PLUS MINUS MULTIPLY DIVIDE MOD ASSIGN LESS LESS_EQUAL NOT_EQUAL GREAT_EQUAL GREAT EQUAL AND OR NOT KW_VAR KW_ARRAY KW_OF KW_BOOLEAN KW_INT KW_REAL KW_STRING KW_TRUE KW_FALSE KW_DEF KW_RETURN KW_BEGIN KW_END KW_WHILE KW_DO KW_IF KW_THEN KW_ELSE KW_FOR KW_TO KW_PRINT KW_READ ID INT OCT_INT FLOAT SCIENTIFIC STRING SPACE
%left AND OR NOT
%left LESS LESS_EQUAL NOT_EQUAL GREAT_EQUAL GREAT EQUAL
%left MINUS
%left PLUS
%left DIVIDE MOD
%left MULTIPLY
%nonassoc UMINUS

%%
Program: Program_Unit | Function_Declaration | Function_Definition;

Program_Unit: ID SEMICOLON Variables Functions Compound_Statement KW_END;

Functions: /* empty */ | Function_Declaration Functions | Function_Definition Functions;
Function_Declaration: Function_Declaration_Prime SEMICOLON;
Function_Declaration_Prime: Function_Declaration_Procedure Scalar_Type;
Scalar_Type: /* empty */ | COLON Scalar_Type_Prime;
Scalar_Type_Prime: KW_INT | KW_REAL | KW_BOOLEAN | KW_STRING ;
Function_Declaration_Procedure: ID L_PARENTHESIS Formal_Args R_PARENTHESIS;
Function_Definition: Function_Declaration_Prime Compound_Statement KW_END;
Formal_Args: /* empty */ | Formal_Args_Prime;
Formal_Args_Prime: Formal_Arg | Formal_Arg SEMICOLON Formal_Args_Prime;
Formal_Arg: Id_List COLON Type;
Id_List: ID | ID COMMA Id_List;

Variables: /* empty */ | Variable Variables;
Variable: KW_VAR Id_List COLON Variable_Prime SEMICOLON;
Variable_Prime: Type | Constant;
Type: Scalar_Type_Prime | KW_ARRAY INT KW_OF Type;
Constant: INT | OCT_INT | FLOAT | SCIENTIFIC | STRING | KW_TRUE | KW_FALSE;

Compound_Statement: KW_BEGIN Variables Statements KW_END;
Statements: /* empty */ | Statement Statements;
Statement: Simple_Statement | Compound_Statement | Conditional_Statement | While_Statement | For_Statement | Return_Statement | Function_Call;
Simple_Statement: Variable_Ref ASSIGN Expression SEMICOLON| KW_PRINT Expression SEMICOLON| KW_READ Variable_Ref SEMICOLON;
Variable_Ref: ID Brackets;
Brackets: /* empty */ | Bracket Brackets;
Bracket: L_BRACKET Expression R_BRACKET;
Conditional_Statement: KW_IF Expression KW_THEN Compound_Statement Else KW_END KW_IF;
Else: /* empty */ | KW_ELSE Compound_Statement;
While_Statement: KW_WHILE Expression KW_DO Compound_Statement KW_END  KW_DO;
For_Statement: KW_FOR ID ASSIGN INT KW_TO INT KW_DO Compound_Statement KW_END KW_DO;
Return_Statement: KW_RETURN Expression SEMICOLON;
Function_Call: Function_Call_Prime SEMICOLON;
Function_Call_Prime: ID L_PARENTHESIS Expressions R_PARENTHESIS;

Expressions: /*empty */ | Expressions_Prime;
Expressions_Prime: Expression | Expression COMMA Expressions_Prime;
Expression:  L_PARENTHESIS Expression R_PARENTHESIS |
            Expression AND Expression | 
            Expression OR Expression | 
            NOT Expression | 
            Expression LESS Expression | 
            Expression LESS_EQUAL Expression | 
            Expression NOT_EQUAL Expression | 
            Expression GREAT_EQUAL Expression | 
            Expression GREAT Expression | 
            Expression EQUAL Expression | 
            Expression MINUS Expression | 
            Expression PLUS Expression | 
            Expression DIVIDE Expression | 
            Expression MOD Expression | 
            Expression MULTIPLY Expression | 
            MINUS Expression %prec UMINUS | 
            Constant | Variable_Ref | Function_Call_Prime; 

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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
