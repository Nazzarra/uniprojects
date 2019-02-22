#ifndef _ASM_CONSTANTS_H
#define _ASM_CONSTANTS_H

#define MAX_INSTR_OPCODE_NAME 32
#define MAX_SYMBOL_NAME 32
#define MAX_ERROR_MSG_LEN 256

#define FIRST_PASS 0
#define SECOND_PASS 1

//20
#define OPERAND_IMM 0
//&x
#define OPERAND_SYMBOL_VALUE 1
//x
#define OPERAND_SYMBOL_MEMDIR 2
//*20
#define OPERAND_ABSOLUTE_MEMDIR 3
//r5
#define OPERAND_REGDIR 4
//r5[32]
#define OPERAND_REGINDIR_ABSOLUTE 5
//r5[x]
#define OPERAND_REGINDIR_SYMBOL 6
//$x
#define OPERAND_RELATIVE_MEMDIR 7

#define DIRECTIVE_ARG_NUMBER 0
#define DIRECTIVE_ARG_SYMBOL 1

#define SECTION_UND 0
#define SECTION_TEXT 1
#define SECTION_DATA 2
#define SECTION_RODATA 3
#define SECTION_BSS 4

#define SECTION_CNT 5

#define LINKAGE_LOCAL 0
#define LINKAGE_GLOBAL 1

#define SYMTYPE_SYM 0
#define SYMTYPE_SECTION 1

#define RELOCATION_PC_RELATIVE 0
#define RELOCATION_ABSOLUTE 1

#define INSTR_COND_EQUAL 0
#define INSTR_COND_NOT_EQUAL 1
#define INSTR_COND_GREATER_THAN 2
#define INSTR_COND_NONE 3

#define INSTR_COND_CNT 4

#define INSTR_BASE_SIZE_BYTES 2
#define INSTR_EXTRA_SIZE_BYTES 4

#define INSTRUCTION_SET_SIZE 18

#define INSTR_INVALID_MNEMONIC 0xFF
#define INSTR_INVALID_COND 0xFF

#define DIRECTIVE_CNT 10
#define DIRECTIVE_INVALID 0xFF

#define BYTES_PER_LINE 16

#define RELOCATION_TABS_CNT 3

#define REGISTER_PC 7

//Instruction mnemonics
//const char* mnemonics[INSTRUCTION_SET_SIZE] = { "add", "sub", "mul", "div", "cmp", "and", "or", "not", "test", "push", "pop", "call", "iret", "mov", "shl", "shr", "jmp", "ret" };
#define MNEMONIC_ADD 0
#define MNEMONIC_SUB 1
#define MNEMONIC_MUL 2
#define MNEMONIC_DIV 3
#define MNEMONIC_CMP 4
#define MNEMONIC_AND 5
#define MNEMONIC_OR 6
#define MNEMONIC_NOT 7
#define MNEMONIC_TEST 8
#define MNEMONIC_PUSH 9
#define MNEMONIC_POP 10
#define MNEMONIC_CALL 11
#define MNEMONIC_IRET 12
#define MNEMONIC_MOV 13
#define MNEMONIC_SHL 14
#define MNEMONIC_SHR 15

#define MNEMONIC_JMP 16
#define MNEMONIC_RET 17

//Directive mnemonics
//const char* directives[DIRECTIVE_CNT] = { "text", "data", "rodata", "bss", "char", "word", "long", "align", "skip", "global"};
#define DIRECTIVE_TEXT 0
#define DIRECTIVE_DATA 1
#define DIRECTIVE_RODATA 2
#define DIRECTIVE_BSS 3
#define DIRECTIVE_CHAR 4
#define DIRECTIVE_WORD 5
#define DIRECTIVE_LONG 6
#define DIRECTIVE_ALIGN 7
#define DIRECTIVE_SKIP 8
#define DIRECTIVE_GLOBAL 9

#endif