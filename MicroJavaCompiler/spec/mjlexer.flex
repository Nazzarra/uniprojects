package rs.ac.bg.etf.pp1;
import java_cup.runtime.Symbol;

%%

%{

    private Symbol newSymbol(int type){
        return new Symbol(type, yyline + 1, yycolumn);
    }

    private Symbol newSymbol(int type, Object value){
        return new Symbol(type, yyline + 1, yycolumn, value);
    }

%}

%eofval{
    return newSymbol(sym.EOF);
%eofval}


%cup
%line
%column

%xstate COMMENT

%%

" "     { }
"\b"    { }
"\f"    { }
"\t"    { }
"\r\n"    { }

"program" { return newSymbol(sym.PROGRAM); }
"break" { return newSymbol(sym.BREAK); }
"class" { return newSymbol(sym.CLASS); }
"interface" { return newSymbol(sym.INTERFACE); }
"enum" { return newSymbol(sym.ENUM); }
"else" { return newSymbol(sym.ELSE); }
"const" { return newSymbol(sym.CONST); }
"if" { return newSymbol(sym.IF); }
"new" { return newSymbol(sym.NEW); }
"print" { return newSymbol(sym.PRINT); }
"read" { return newSymbol(sym.READ); }
"return" { return newSymbol(sym.RETURN); }
"void" { return newSymbol(sym.VOID); }
"for" { return newSymbol(sym.FOR); }
"extends" { return newSymbol(sym.EXTENDS); }
"implements" { return newSymbol(sym.IMPLEMENTS); }
"continue" { return newSymbol(sym.CONTINUE); }

[0-9]+ { return newSymbol(sym.NUM_CONST, new Integer(yytext())); }
'.' { return newSymbol(sym.CHAR_CONST, yytext().charAt(1)); }
"true" { return newSymbol(sym.BOOL_CONST, true); }
"false" { return newSymbol(sym.BOOL_CONST, false); }
[a-zA-Z][a-zA-Z0-9_]* { return newSymbol(sym.IDENTIFIER, yytext()); }

"+" { return newSymbol(sym.OP_PLUS); }
"-" { return newSymbol(sym.OP_MINUS); }
"*" { return newSymbol(sym.OP_MUL); }
"/" { return newSymbol(sym.OP_DIV); }
"%" { return newSymbol(sym.OP_MOD); }
"==" { return newSymbol(sym.OP_EQUALS); }
"!=" { return newSymbol(sym.OP_NOT_EQUALS); }
">" { return newSymbol(sym.OP_GREATER_THAN); }
">=" { return newSymbol(sym.OP_GREATER_EQUAL); }
"<" { return newSymbol(sym.OP_LESS_THAN); }
"<=" { return newSymbol(sym.OP_LESS_EQUAL); }
"&&" { return newSymbol(sym.OP_AND); }
"||" { return newSymbol(sym.OP_OR); }
"=" { return newSymbol(sym.OP_ASSIGN); }
"++" { return newSymbol(sym.OP_INC); }
"--" { return newSymbol(sym.OP_DEC); }
";" { return newSymbol(sym.OP_SEMICOLON); }
"," { return newSymbol(sym.OP_COMMA); }
"." { return newSymbol(sym.OP_DOT); }
"(" { return newSymbol(sym.OP_LEFT_PAREN); }
")" { return newSymbol(sym.OP_RIGHT_PAREN); }
"[" { return newSymbol(sym.OP_LEFT_SQUARE); }
"]" { return newSymbol(sym.OP_RIGHT_SQUARE); }
"{" { return newSymbol(sym.OP_LEFT_CURLY); }
"}" { return newSymbol(sym.OP_RIGHT_CURLY); }

"//" { yybegin(COMMENT); }
<COMMENT> . { }
<COMMENT> "\n" { yybegin(YYINITIAL); }

. { System.err.println("Unexpected character ''"+ yytext() + "' encountered at line " + (yyline + 1) + " column " + yycolumn); }