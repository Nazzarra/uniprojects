#include "asm_globals.h"

FILE* current_file;
const char* current_file_path;

uint32_t current_pass;

FILE* output_file_text;
FILE* output_file_bin;

struct symtab* symbol_table;
uint32_t symbol_cnt, symbol_table_size;

struct reltab section_reltab[RELOCATION_TABS_CNT];

uint32_t current_section = SECTION_UND;
uint16_t section_loc = 0;
uint16_t section_start_address = 0;

const char* mnemonics[INSTRUCTION_SET_SIZE] = { "add", "sub", "mul", "div", "cmp", "and", "or", "not", "test", "push", "pop", "call", "iret", "mov", "shl", "shr", "jmp", "ret" };
const uint8_t instr_op_cnt[INSTRUCTION_SET_SIZE] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 0, 2, 2, 2, 1, 0 };

const char* instr_cond_suffix[INSTR_COND_CNT] = { "eq", "ne", "gt", "al" };

const char* directives[DIRECTIVE_CNT] = { "text", "data", "rodata", "bss", "char", "word", "long", "align", "skip", "global"};

const char* sections[] = {".UND", ".TEXT", ".DATA", ".RODATA", ".BSS"};

uint8_t assembly_failed;

uint32_t output_byte_cnt;

//Controls which operand is coded as destination and which as source, has no effect on 1 or 0 arg instructions
uint8_t operand_dst_first = 1;