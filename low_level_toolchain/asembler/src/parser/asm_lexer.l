%{
#include <stdio.h>
#define YY_DECL int yylex()

#include "asm_parser.tab.h"  // to get the token types that we return

static int num_to_hex(const char* text);


%}
%option noyywrap
%%

[ \t]                           ;
[a-zA-Z][a-zA-Z\-_0-9]*         { yylval.tokString = strdup(yytext); return IDENTIFIER; }
,                               { return COMMA; }
:                               { return COLON; }
\n                              { return NEW_LINE; }
\.                              { return PERIOD; }
\[                              { return LEFT_SQUARE_BRACE; }
\]                              { return RIGHT_SQUARE_BRACE; }
\*                              { return ASTERISK; }
\&                              { return AMPERSAND; }
'.'                             { yylval.tokNum = yytext[1]; return NUMBER; }
0x[0-9a-fA-F]+                  { yylval.tokNum = num_to_hex(yytext + 2); return NUMBER; }
-[0-9]+                         { yylval.tokNum = -atoi(yytext + 1); return NUMBER; }
[0-9]+                          { yylval.tokNum = atoi(yytext); return NUMBER; }
\$                              { return DOLLAR; }
<<EOF>>                         { static int once = 0; return (once = !once) ? NEW_LINE : 0; }
. 			               		{ YY_FLUSH_BUFFER; printf("Lexer has encountered an invalid character: %s\n", yytext); return ERROR_TOK; };
%%

static int num_to_hex(const char* text)
{
    int res = 0;
    int i = 0;
    while(text[i] != '\0'){
        int digit;
        if(text[i] >= '0' && text[i] <= '9')
            digit = text[i] - '0';
        else if(text[i] >= 'A' && text[i] <= 'F')
            digit = text[i] - 'A' + 10;
        else
            digit = text[i] - 'a' + 10;
        
        res = res * 16 + digit;
        ++i;
    }
    return res;
}