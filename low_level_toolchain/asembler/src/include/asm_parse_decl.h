#ifndef _ASM_PARSE_DECL_H
#define _ASM_PARSE_DECL_H

#include <stdint.h>
#include "asm_constants.h"

extern char error_msg_buffer[MAX_ERROR_MSG_LEN];
struct op_imm
{
    uint16_t value;
};

struct op_symval
{
    char symname[MAX_SYMBOL_NAME];
};

struct op_symmem
{
    char symname[MAX_SYMBOL_NAME];
};

struct op_absmem
{
    uint16_t addr;
};

struct op_regdir
{
    uint8_t reg;
};

struct op_regindir
{
    uint8_t reg;
    uint16_t disp;
};

struct op_regindirsym
{
    uint8_t reg;
    char symname[MAX_SYMBOL_NAME];
};

struct op_rel_memdir
{
    char symname[MAX_SYMBOL_NAME];
};

struct operand_info
{
    uint32_t type;
    union
    {
        struct op_imm imm;
        struct op_symval symval;
        struct op_symmem symmem;
        struct op_absmem absmem;
        struct op_regdir regdir;
        struct op_regindir regindir;
        struct op_regindirsym regindirsym;
        struct op_rel_memdir rel_memdir;
    };
};

struct instruction_info
{
    uint8_t mnemonic_id;
    uint8_t condition_id;
    int arg_cnt;
    struct operand_info first_op;
    struct operand_info second_op;
};

struct directive_arg
{
    uint32_t type;
    union
    {
        uint32_t num;
        char symname[MAX_SYMBOL_NAME];
    };
    struct directive_arg* next;
};

struct directive_info
{
    uint8_t directive_id;
    uint32_t arg_cnt;
    struct directive_arg* args; //Must be freed
};

int asm_parseh_isregident(const char* identifier);
int asm_parseh_getregfromident(const char* identifier);

int asm_parseh_is_valid_mnemonic(const char* identifier);
void asm_parseh_extract_mnemonic(struct instruction_info* instr_info, const char* identifier);

int asm_parseh_is_valid_directive(const char* identifier);
int asm_parseh_get_directive_id(const char* identifier);

void asm_parseh_error();

void asm_parseh_copy_opinfo(struct operand_info* dst, const struct operand_info* src);

void asm_parseh_copy_directive_arg(struct directive_arg* dst, const struct directive_arg* src);
void asm_parseh_free_directive_args(struct directive_arg* arg_head);

void asm_parseh_str_tolower(char* string);

#endif