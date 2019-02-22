%{
    #include <stdio.h>
    #include <string.h>
    #include "asm_api.h"

    #define MAX_ERROR_MSG_LEN 256

    int yylex();
    int yyparse();
    FILE *yyin;
    
    void yyerror(const char *s);

    extern char error_msg_buffer[MAX_ERROR_MSG_LEN];
    extern uint32_t current_pass;
%}

%code requires {
    #include "asm_parse_decl.h"
    //For some reason, including in the first section is not enough for the types to be visible for the %union
    //This does the job
}

%union {
    int tokNum;
    char tokChar;
    char *tokString;
    char label[MAX_SYMBOL_NAME];
    struct directive_info dir_info;
    struct instruction_info instr_info;
    struct operand_info operand;
    struct directive_arg dir_arg;
}


//Every possible token
%token <tokNum> NUMBER
%token <tokString> IDENTIFIER
%token <tokChar> COLON
%token <tokChar> NEW_LINE
%token <tokChar> COMMA
%token <tokChar> PERIOD 
%token <tokChar> LEFT_SQUARE_BRACE
%token <tokChar> RIGHT_SQUARE_BRACE
%token <tokChar> ASTERISK
%token <tokChar> AMPERSAND
%token <tokChar> DOLLAR
%token ERROR_TOK

//Instruction types 
%type <label> label
%type <dir_info> directive
%type <dir_arg> arg
%type <instr_info> no_arg_instruction
%type <instr_info> one_arg_instruction
%type <instr_info> two_arg_instruction
%type <operand> operand

%%

line:
    line_base
    {
        YYACCEPT;
    }
    | 
    label line_base
    {
        YYACCEPT;
    }
    |
    {
        YYABORT;
    }
    ;

line_base:
    NEW_LINE 
    | directive NEW_LINE
    | no_arg_instruction NEW_LINE
    | one_arg_instruction NEW_LINE
    | two_arg_instruction NEW_LINE
    ;

label:
    IDENTIFIER COLON 
    { 
        if(current_pass == FIRST_PASS){
            if(!asm_fp_label($1)){
                asm_parseh_error();
                YYABORT;
            }
        }
        //Labels do not need a second pass
    }
    ; 

directive:
    PERIOD IDENTIFIER
    {
        asm_parseh_str_tolower($2);
        /* .end means we stop parsing */
        if(strcmp($2, "end") == 0){
            YYABORT;
        }

        if(!asm_parseh_is_valid_directive($2)){
            sprintf(error_msg_buffer, "Invalid directive: %s", $2);
            asm_parseh_error();
            YYABORT;
        }
        $$.directive_id = asm_parseh_get_directive_id($2);
        $$.args = NULL;

        if(current_pass == FIRST_PASS){
            if(!asm_fp_directive(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
        else{
            if(!asm_sp_directive(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
    }
    | 
    PERIOD IDENTIFIER arg
    {
        asm_parseh_str_tolower($2);
        if(!asm_parseh_is_valid_directive($2)){
            sprintf(error_msg_buffer, "Invalid directive: %s", $2);
            asm_parseh_error();
            YYABORT;
        }
        $$.directive_id = asm_parseh_get_directive_id($2);
        $$.args = &$3;
        
        if(current_pass == FIRST_PASS){
            if(!asm_fp_directive(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
        else{
           if(!asm_sp_directive(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
    } 
    ;

arg:
    /* Directive arg: symbol memdir */
    IDENTIFIER 
    { 
        $$.type = DIRECTIVE_ARG_SYMBOL;
        strcpy($$.symname, $1);
        $$.next = NULL;
    }
    | 
    /* Directive arg: number */
    NUMBER 
    {
        $$.type = DIRECTIVE_ARG_NUMBER;
        $$.num = $1;
        $$.next = NULL;
    }
    | 
    IDENTIFIER COMMA arg
    {
        struct directive_arg* tmp;
        $$.type = DIRECTIVE_ARG_SYMBOL;
        strcpy($$.symname, $1);
        tmp = malloc(sizeof(struct directive_arg));
        asm_parseh_copy_directive_arg(tmp, &$3);
        $$.next = tmp;
    }
    | 
    NUMBER COMMA arg
    {
        struct directive_arg* tmp;
        $$.type = DIRECTIVE_ARG_NUMBER;
        $$.num = $1;
        tmp = malloc(sizeof(struct directive_arg));
        asm_parseh_copy_directive_arg(tmp, &$3);
        $$.next = tmp;
    } 
    ;

no_arg_instruction:
    IDENTIFIER
    {
        asm_parseh_str_tolower($1);
        if(!asm_parseh_is_valid_mnemonic($1)){
            sprintf(error_msg_buffer, "Invalid instruction mnemonic: %s", $1);
            asm_parseh_error();
            YYABORT;
        }
        asm_parseh_extract_mnemonic(&$$, $1);
        $$.arg_cnt = 0;
        
        if(current_pass == FIRST_PASS){
            if(!asm_fp_instruction(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
        else{
            if(!asm_sp_instruction(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
    }
    ;

one_arg_instruction:
    IDENTIFIER operand
    {
        asm_parseh_str_tolower($1);
        if(!asm_parseh_is_valid_mnemonic($1)){
            sprintf(error_msg_buffer, "Invalid instruction mnemonic: %s", $1);
            asm_parseh_error();
            YYABORT;
        }
        asm_parseh_extract_mnemonic(&$$, $1);
        $$.arg_cnt = 1;
        asm_parseh_copy_opinfo(&$$.first_op, &$2);
        
        if(current_pass == FIRST_PASS){
            if(!asm_fp_instruction(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
        else{
            if(!asm_sp_instruction(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
    }
    ;

two_arg_instruction:
    IDENTIFIER operand COMMA operand
    {
        asm_parseh_str_tolower($1);
        if(!asm_parseh_is_valid_mnemonic($1)){
            sprintf(error_msg_buffer, "Invalid instruction mnemonic: %s", $1);
            asm_parseh_error();
            YYABORT;
        }
        asm_parseh_extract_mnemonic(&$$, $1);
        $$.arg_cnt = 2;
        asm_parseh_copy_opinfo(&$$.first_op, &$2);
        asm_parseh_copy_opinfo(&$$.second_op, &$4);

        if(current_pass == FIRST_PASS){
            if(!asm_fp_instruction(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
        else{
            if(!asm_sp_instruction(&$$)){
                asm_parseh_error();
                YYABORT;
            }
        }
    }
    ;

operand:
    /* Immediate operand */ 
    NUMBER 
    { 
        $$.type = OPERAND_IMM; 
        $$.imm.value = $1; 
    }
    |
    /* Register indirect by constant */
    IDENTIFIER LEFT_SQUARE_BRACE NUMBER RIGHT_SQUARE_BRACE
    {
        if(!asm_parseh_isregident($1)){
            sprintf(error_msg_buffer, "Expected register identifier, found %s", $1);
            asm_parseh_error();
            YYABORT;
        }

        $$.type = OPERAND_REGINDIR_ABSOLUTE;
        $$.regindir.disp = $3;
        $$.regindir.reg = asm_parseh_getregfromident($1);
    }
    |
    /* Register indirect by symbol value or PSW register direct*/
    IDENTIFIER LEFT_SQUARE_BRACE IDENTIFIER RIGHT_SQUARE_BRACE
    {
        if(!asm_parseh_isregident($1)){
            sprintf(error_msg_buffer, "Expected register identifier, found %s", $1);
            asm_parseh_error();
            YYABORT;
        }

        
        $$.type = OPERAND_REGINDIR_SYMBOL;
        strcpy($$.regindirsym.symname, $3);
        $$.regindirsym.reg = asm_parseh_getregfromident($1);
    
    }
    | 
    /* Immediate by label value */
    AMPERSAND IDENTIFIER 
    { 
        $$.type = OPERAND_SYMBOL_VALUE;
        strcpy($$.symval.symname, $2);
    }
    | 
    /* Memory direct by label value  or register direct */
    IDENTIFIER 
    { 
        //Register direct
        if(asm_parseh_isregident($1)){
            $$.type = OPERAND_REGDIR;
            $$.regdir.reg = asm_parseh_getregfromident($1);
        }
        else{
            //Register direct PSW
            if(strcmp($1, "PSW") == 0){
                $$.type = OPERAND_REGDIR;
                $$.regdir.reg = 8;
            }
            //Memory direct by symbol
            else{
                $$.type = OPERAND_SYMBOL_MEMDIR;
                strcpy($$.symmem.symname, $1);
            }
        }
    }
    | 
    /* Memory direct by location */
    ASTERISK NUMBER
    {
        if($2 < 0 || $2 > ( (1 << (sizeof($$.absmem.addr) * 8)) - 1 )){
            sprintf(error_msg_buffer, "Constant %d bigger than 2 bytes", $2);
            asm_parseh_error();
            YYABORT;
        }
        $$.type = OPERAND_ABSOLUTE_MEMDIR;
        $$.absmem.addr = $2;
    }
    | 
    /* Memory relative by symbol */
    DOLLAR IDENTIFIER
    {
        $$.type = OPERAND_RELATIVE_MEMDIR;
        strcpy($$.rel_memdir.symname, $2);
    }
%%