#include "asm_parse_decl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "asm_globals.h"

char error_msg_buffer[MAX_ERROR_MSG_LEN];

extern int line_number;

void asm_parseh_str_tolower(char* string)
{
    while(*string != '\0'){
        *string = tolower(*string);
        ++string;
    }
}

/* When a token is for register access, it is of the format:
    r<number> */
int asm_parseh_isregident(const char* identifier)
{
    int i;
    for(i = 0; i < 2; ++i)
        if(identifier[i] == '\0')
            return 0;
    
    if(identifier[2] != '\0')
        return 0;
    
    if(identifier[0] != 'r' && identifier[0] != 'R')
        return 0;
    
    int reg = identifier[1] - '0';
    if(reg < 0 || reg > 7)
        return 0;
    
    return 1;
}

int asm_parseh_getregfromident(const char* identifier)
{
   return identifier[1] - '0'; 
}


void asm_parseh_error()
{
    printf("Error(Line %d): %s\n", line_number, error_msg_buffer);
    assembly_failed = 1;
}

void asm_parseh_copy_opinfo(struct operand_info* dst, const struct operand_info* src)
{
    dst->type = src->type;
    if(src->type == OPERAND_IMM){
        dst->imm.value = src->imm.value;
    }
    else if(src->type == OPERAND_SYMBOL_VALUE){
        strcpy(dst->symval.symname, src->symval.symname);
    }
    else if(src->type == OPERAND_SYMBOL_MEMDIR){
        strcpy(dst->symmem.symname, src->symmem.symname);
    }
    else if(src->type == OPERAND_ABSOLUTE_MEMDIR){
        dst->absmem.addr = src->absmem.addr;
    }
    else if(src->type == OPERAND_REGDIR){
        dst->regdir.reg = src->regdir.reg;
    }
    else if(src->type == OPERAND_REGINDIR_ABSOLUTE){
        dst->regindir.reg = src->regindir.reg;
        dst->regindir.disp = src->regindir.disp;
    }
    else if(src->type == OPERAND_REGINDIR_SYMBOL){
        dst->regindirsym.reg = src->regindirsym.reg;
        strcpy(dst->regindirsym.symname, src->regindirsym.symname);
    }
    else if(src->type == OPERAND_RELATIVE_MEMDIR){
        strcpy(dst->rel_memdir.symname, src->rel_memdir.symname);
    }
    else{
        printf("Unrecoverable operand copy error.");
        exit(666);
    }
}

void asm_parseh_copy_directive_arg(struct directive_arg* dst, const struct directive_arg* src)
{
    dst->type = src->type;
    if(src->type == DIRECTIVE_ARG_NUMBER){
        dst->num = src->num;
    }
    else if(src->type == DIRECTIVE_ARG_SYMBOL){
        strcpy(dst->symname, src->symname);
    }
    else{
        printf("Unrecoverable directive arg copy error.");
        exit(666);
    }
    dst->next = src->next;
}

void asm_parseh_free_directive_args(struct directive_arg* arg_head)
{
    if(arg_head != NULL){
        asm_parseh_free_directive_args(arg_head->next);
        free(arg_head);
    }
}

static uint8_t asm_parseh_get_mnemonic_id(const char* identifier)
{
    for(int i = 0; i < INSTRUCTION_SET_SIZE; ++i){
        int len = strlen(mnemonics[i]);
        if(strncmp(identifier, mnemonics[i], len) == 0)
            return i;
    }

    return INSTR_INVALID_MNEMONIC;
}

static int asm_parseh_get_condition_id(const char* identifier)
{
    uint8_t mnemonic_id = asm_parseh_get_mnemonic_id(identifier);
    const char* cond_str = identifier + (strlen(mnemonics[mnemonic_id]));

    if(cond_str[0] == '\0')
        return INSTR_COND_NONE;
    if(cond_str[1] == '\0' || cond_str[2] != '\0')
        return INSTR_INVALID_COND;
    
    for(int i = 0; i < INSTR_COND_CNT; ++i){
        if(strcmp(cond_str, instr_cond_suffix[i]) == 0)
            return i;
    }

    return INSTR_INVALID_COND;
}

int asm_parseh_is_valid_mnemonic(const char* identifier)
{
    uint8_t mnemonic_id = asm_parseh_get_mnemonic_id(identifier);
    if(mnemonic_id == INSTR_INVALID_MNEMONIC)
        return 0;

    uint8_t condition_id = asm_parseh_get_condition_id(identifier);
    return condition_id != INSTR_INVALID_COND;
}

void asm_parseh_extract_mnemonic(struct instruction_info* instr_info, const char* identifier)
{
    instr_info->mnemonic_id = asm_parseh_get_mnemonic_id(identifier);
    instr_info->condition_id = asm_parseh_get_condition_id(identifier);
}

int asm_parseh_get_directive_id(const char* identifier)
{
    for(int i = 0; i < DIRECTIVE_CNT; ++i){
        if(strcmp(identifier, directives[i]) == 0)
            return i;
    }

    return DIRECTIVE_INVALID;
}

int asm_parseh_is_valid_directive(const char* identifier)
{
    uint8_t directive_id = asm_parseh_get_directive_id(identifier);
    return directive_id != DIRECTIVE_INVALID;
}