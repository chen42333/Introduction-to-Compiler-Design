%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512

uint32_t line_num = 1;
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
"+"   { LIST_TOKEN("+"); return PLUS; }
"-"   { LIST_TOKEN("-"); return MINUS; }
"*"   { LIST_TOKEN("*"); return MULTIPLY; }
"/"   { LIST_TOKEN("/"); return DIVIDE; }
"mod" { LIST_TOKEN("mod"); return MOD; }
":="  { LIST_TOKEN(":="); return ASSIGN; }
"<"   { LIST_TOKEN("<"); return LESS; }
"<="  { LIST_TOKEN("<="); return LESS_EQUAL; }
"<>"  { LIST_TOKEN("<>"); return NOT_EQUAL; }
">="  { LIST_TOKEN(">="); return GREAT_EQUAL; }
">"   { LIST_TOKEN(">"); return GREAT; }
"="   { LIST_TOKEN("="); return EQUAL; }
"and" { LIST_TOKEN("and"); return AND; }
"or"  { LIST_TOKEN("or"); return OR; }
"not" { LIST_TOKEN("not"); return NOT; }

    /* Reserved Word */
"var"     { LIST_TOKEN("KWvar"); return KW_VAR; }
"array"   { LIST_TOKEN("KWarray"); return KW_ARRAY; }
"of"      { LIST_TOKEN("KWof"); return KW_OF; }
"boolean" { LIST_TOKEN("KWboolean"); return KW_BOOLEAN; }
"integer" { LIST_TOKEN("KWinteger"); return KW_INT; }
"real"    { LIST_TOKEN("KWreal"); return KW_REAL; }
"string"  { LIST_TOKEN("KWstring"); return KW_STRING; }

"true"    { LIST_TOKEN("KWtrue"); return KW_TRUE; }
"false"   { LIST_TOKEN("KWfalse"); return KW_FALSE; }

"def"     { LIST_TOKEN("KWdef"); return KW_DEF; }
"return"  { LIST_TOKEN("KWreturn"); return KW_RETURN; }

"begin"   { LIST_TOKEN("KWbegin"); return KW_BEGIN; }
"end"     { LIST_TOKEN("KWend"); return KW_END; }

"while"   { LIST_TOKEN("KWwhile"); return KW_WHILE; }
"do"      { LIST_TOKEN("KWdo"); return KW_DO; }

"if"      { LIST_TOKEN("KWif"); return KW_IF; }
"then"    { LIST_TOKEN("KWthen"); return KW_THEN; }
"else"    { LIST_TOKEN("KWelse"); return KW_ELSE; }

"for"     { LIST_TOKEN("KWfor"); return KW_FOR; }
"to"      { LIST_TOKEN("KWto"); return KW_TO; }

"print"   { LIST_TOKEN("KWprint"); return KW_PRINT; }
"read"    { LIST_TOKEN("KWread"); return KW_READ; }

    /* Identifier */
[a-zA-Z][a-zA-Z0-9]* { LIST_LITERAL("id", yytext); return ID; }

    /* Integer (decimal/octal) */
{integer} { LIST_LITERAL("integer", yytext); return INT; }
0[0-7]+   { LIST_LITERAL("oct_integer", yytext); return OCT_INT; }

    /* Floating-Point */
{float} { LIST_LITERAL("float", yytext); return FLOAT; }

    /* Scientific Notation [Ee][+-]?[0-9]+ */
({nonzero_integer}|{nonzero_float})[Ee][+-]?({integer}) { LIST_LITERAL("scientific", yytext); return SCIENTIFIC; }

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
    return STRING;
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
