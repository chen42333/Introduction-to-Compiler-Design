%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define YY_USER_ACTION \
    yylloc.first_line = line_num; \
    yylloc.first_column = col_num; \
    col_num += yyleng;

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512
#define MAX_ID_LENG                 32

// prevent undefined reference error in newer version of flex
extern "C" int yylex(void);

uint32_t line_num = 1;
uint32_t col_num = 1;
char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char string_literal[MAX_LINE_LENG];
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);

%}

integer 0|[1-9][0-9]*
float {integer}\.(0|[0-9]*[1-9])
nonzero_integer [1-9][0-9]*
    /* If the integral part is nonzero, then the fractional part can be either zero
    or not. However, if the integral part is zero, the fractional part cannot be zero. */
nonzero_float ({nonzero_integer}\.(0|[0-9]*[1-9]))|(0\.([0-9]*[1-9]))

    /* exclusive start condition */
%x CCOMMENT

%%
    /* Delimiter */
"," { LIST_TOKEN(","); return COMMA; }
";" { LIST_TOKEN(";"); return SEMICOLON; }
":" { LIST_TOKEN(":"); return COLON; }
"(" { LIST_TOKEN("("); return L_PARENTHESIS; }
")" { LIST_TOKEN(")"); return R_PARENTHESIS; }
"[" { LIST_TOKEN("["); return L_BRACKET; }
"]" { LIST_TOKEN("]"); return R_BRACKET; }

    /* Operator */
"+"   { LIST_TOKEN("+"); yylval.op = strdup(yytext); return PLUS; }
"-"   { LIST_TOKEN("-"); yylval.op = strdup(yytext); return MINUS; }
"*"   { LIST_TOKEN("*"); yylval.op = strdup(yytext);return MULTIPLY; }
"/"   { LIST_TOKEN("/"); yylval.op = strdup(yytext); return DIVIDE; }
"mod" { LIST_TOKEN("mod"); yylval.op = strdup(yytext); return MOD; }
":="  { LIST_TOKEN(":="); yylval.op = strdup(yytext); return ASSIGN; }
"<"   { LIST_TOKEN("<"); yylval.op = strdup(yytext); return LESS; }
"<="  { LIST_TOKEN("<="); yylval.op = strdup(yytext); return LESS_OR_EQUAL; }
"<>"  { LIST_TOKEN("<>"); yylval.op = strdup(yytext); return NOT_EQUAL; }
">="  { LIST_TOKEN(">="); yylval.op = strdup(yytext); return GREATER_OR_EQUAL; }
">"   { LIST_TOKEN(">"); yylval.op = strdup(yytext); return GREATER; }
"="   { LIST_TOKEN("="); yylval.op = strdup(yytext); return EQUAL; }
"and" { LIST_TOKEN("and"); yylval.op = strdup(yytext); return AND; }
"or"  { LIST_TOKEN("or"); yylval.op = strdup(yytext);return OR; }
"not" { LIST_TOKEN("not"); yylval.op = strdup(yytext); return NOT; }

    /* Reserved Word */
"var"     { LIST_TOKEN("KWvar"); return VAR; }
"array"   { LIST_TOKEN("KWarray"); return ARRAY; }
"of"      { LIST_TOKEN("KWof"); return OF; }
"boolean" { LIST_TOKEN("KWboolean"); yylval.string_ = "boolean"; return BOOLEAN; }
"integer" { LIST_TOKEN("KWinteger"); yylval.string_ = "integer"; return INTEGER; }
"real"    { LIST_TOKEN("KWreal"); yylval.string_ = "real"; return REAL; }
"string"  { LIST_TOKEN("KWstring"); yylval.string_ = "string"; return STRING; }

"true"    { LIST_TOKEN("KWtrue"); return TRUE; }
"false"   { LIST_TOKEN("KWfalse"); return FALSE; }

"def"     { LIST_TOKEN("KWdef"); return DEF; }
"return"  { LIST_TOKEN("KWreturn"); return RETURN; }

"begin"   { LIST_TOKEN("KWbegin"); return BEGIN_; }
"end"     { LIST_TOKEN("KWend"); return END; }

"while"   { LIST_TOKEN("KWwhile"); return WHILE; }
"do"      { LIST_TOKEN("KWdo"); return DO; }

"if"      { LIST_TOKEN("KWif"); return IF; }
"then"    { LIST_TOKEN("KWthen"); return THEN; }
"else"    { LIST_TOKEN("KWelse"); return ELSE; }

"for"     { LIST_TOKEN("KWfor"); return FOR; }
"to"      { LIST_TOKEN("KWto"); return TO; }

"print"   { LIST_TOKEN("KWprint"); return PRINT; }
"read"    { LIST_TOKEN("KWread"); return READ; }

    /* Identifier */
[a-zA-Z][a-zA-Z0-9]* { LIST_LITERAL("id", yytext); yylval.identifier = strdup(yytext); return ID; }

    /* Integer (decimal/octal) */
{integer} { LIST_LITERAL("integer", yytext); yylval.number = atoi(yytext); return INT_LITERAL; }
0[0-7]+   { 
    LIST_LITERAL("oct_integer", yytext); 
    yylval.number = yytext[1] - 48; 
    for(int i = 2; yytext[i] != '\0'; i++){
        yylval.number *= 8;
        yylval.number += yytext[i] - 48;
    }
    return INT_LITERAL; }

    /* Floating-Point */
{float} { LIST_LITERAL("float", yytext); yylval.floating = atof(yytext); return REAL_LITERAL; }

    /* Scientific Notation [Ee][+-]?[0-9]+ */
({nonzero_integer}|{nonzero_float})[Ee][+-]?({integer}) { LIST_LITERAL("scientific", yytext); yylval.number = atof(yytext); return REAL_LITERAL; }

    /* String */
\"([^"\n]|\"\")*\" {
    char *yyt_ptr = yytext + 1;  // +1 for skipping the first double quote "
    char *str_ptr = string_literal;

    while (*yyt_ptr) {
        if (*yyt_ptr == '"') {
            // Handle the situation of two double quotes "" in string literal
            if (*(yyt_ptr + 1) == '"') {
                *str_ptr = *yyt_ptr;
                ++str_ptr;
                yyt_ptr += 2; // move to the next character of ""
            } else {
                ++yyt_ptr;
            }
        } else {  // normal character
            *str_ptr = *yyt_ptr;
            ++str_ptr;
            ++yyt_ptr;
        }
    }
    *str_ptr = '\0';
    LIST_LITERAL("string", string_literal);
    yylval.string_ = strdup(string_literal);
    return STRING_LITERAL;
}

    /* Whitespace */
[ \t]+ { LIST_SOURCE; }

    /* Pseudocomment */
"//&"[ST][+-].* {
    LIST_SOURCE;
    char option = yytext[3];
    switch (option) {
    case 'S':
        opt_src = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'T':
        opt_tok = (yytext[4] == '+') ? 1 : 0;
        break;
    }
}

    /* C++ Style Comment */
"//".* { LIST_SOURCE; }

    /* C Style Comment */
"/*"           { LIST_SOURCE; BEGIN(CCOMMENT); }
<CCOMMENT>"*/" { LIST_SOURCE; BEGIN(INITIAL); }
<CCOMMENT>.    { LIST_SOURCE; }

    /* Newline */
<INITIAL,CCOMMENT>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    ++line_num;
    col_num = 1;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}

    /* Catch the character which is not accepted by all rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

static void appendToCurrentLine(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *current_line_ptr = *yytext_ptr;
        ++current_line_ptr;
        ++yytext_ptr;
    }
    *current_line_ptr = '\0';
}
