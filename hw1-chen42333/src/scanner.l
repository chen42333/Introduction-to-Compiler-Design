%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512

uint32_t line_num = 1;
static char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);

%}

delimiter       ,|;|:|"("|")"|"["|"]"
arithmetic      "+"|"-"|"*"|"/"|mod|":="
relation        "<"|"<="|"<>"|">="|">"|=
logical_op      and|or|not
reserved_word   var|def|array|of|boolean|integer|real|string|true|false|while|do|if|then|else|for|to|begin|end|print|read|return
identifier      [a-zA-Z][a-zA-Z0-9]*
octal           0[0-7]*
decimal_pos     [1-9][0-9]*
decimal         0|{decimal_pos}
decimal_float   ({decimal}\.[0-9]*[1-9])|({decimal}\.0)
string          \"[^(\"|\n)]*\"
string_2        {string}+
scientific      (({decimal}\.[0-9]*[1-9])|({decimal_pos}(\.0)?))[e|E][-+]?{decimal}
space           " "|"\t"
comment         "//".*
discarded       {space}|{comment}
source_on       "//&S+".*
source_off      "//&S-".*
token_on        "//&T+".*
token_off       "//&T-".*

%Start COMMENT

%%
<INITIAL>"/*" {
    LIST_SOURCE;
    BEGIN COMMENT;
}
<COMMENT>. { LIST_SOURCE; }
<COMMENT>"*/" {
    LIST_SOURCE;
    BEGIN INITIAL;
}
<INITIAL>{source_on} {
    opt_src = 1;
    LIST_SOURCE;
}
<INITIAL>{source_off} {
    opt_src = 0;
    LIST_SOURCE;
}
<INITIAL>{token_on} {
    opt_tok = 1;
    LIST_SOURCE;
}
<INITIAL>{token_off} {
    opt_tok = 0;
    LIST_SOURCE;
}
<INITIAL>{discarded} { LIST_SOURCE; }
<INITIAL>{string_2} { 
    char* tmp = malloc(yyleng-2+1);
    for(int i = 1, j = 0; ; i++, j++){
        tmp[j] = yytext[i];
        if(yytext[i] == '"' && yytext[i+1] == '"') i++;
        if(yytext[i] == '"' && yytext[i+1] == '\0'){
            tmp[j] = '\0';
            break;
        }
    }
    LIST_LITERAL("string", tmp);
    free(tmp);
}
<INITIAL>{delimiter}|{arithmetic}|{relation}|{logical_op} { LIST_TOKEN(yytext); }
<INITIAL>{reserved_word} {
    char* tmp = malloc(yyleng+3);
    sprintf(tmp, "KW%s", yytext);
    tmp[yyleng+2] = '\0';
    LIST_TOKEN(tmp);
    free(tmp);
}
<INITIAL>{identifier} { LIST_LITERAL("id", yytext); }
<INITIAL>{scientific} { LIST_LITERAL("scientific", yytext); }
<INITIAL>{decimal} { LIST_LITERAL("integer", yytext); }
<INITIAL>{decimal_float} { LIST_LITERAL("float", yytext); }
<INITIAL>{octal} { LIST_LITERAL("oct_integer", yytext); }

\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    ++line_num;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}
    /* Catch the character which is not accepted by rules above */
<INITIAL>. {
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

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}
