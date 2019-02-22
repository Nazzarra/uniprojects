#ifndef _ASM_API_H
#define _ASM_API_H

#include "asm_parse_decl.h"

int asm_do_file (const char* file, uint16_t section_start_address);

int asm_fp_label (char* label_name);
int asm_fp_directive (struct directive_info* dir_info);
int asm_fp_instruction (struct instruction_info* instr_info);

int asm_sp_directive(struct directive_info* dir_info);
int asm_sp_instruction(struct instruction_info* instr_info);

#endif