#ifndef _ASM_GLOBALS_H
#define _ASM_GLOBALS_H

#include "asm_constants.h"
#include <stdio.h>
#include <stdint.h>
#include "asm_table_decl.h"

extern FILE* current_file;
extern const char* current_file_path;

extern FILE* output_file_text;
extern FILE* output_file_bin;

extern int line_number;
extern FILE* yyin;
extern uint32_t current_pass;

extern struct symtab* symbol_table;
extern uint32_t symbol_cnt, symbol_table_size;

extern struct reltab section_reltab[RELOCATION_TABS_CNT];

extern const char* sections[];
extern uint32_t current_section;
extern uint16_t section_loc;
extern uint16_t section_start_address;

extern const char* mnemonics[INSTRUCTION_SET_SIZE];
extern const uint8_t instr_op_cnt[INSTRUCTION_SET_SIZE];

extern const char* instr_cond_suffix[INSTR_COND_CNT];

extern const char* directives[DIRECTIVE_CNT];

extern uint8_t assembly_failed;

extern uint32_t output_byte_cnt;

//Controls which operand is coded as destination and which as source, has no effect on 1 or 0 arg instructions
extern uint8_t operand_dst_first;


#endif