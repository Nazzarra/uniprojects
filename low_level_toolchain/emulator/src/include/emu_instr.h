#ifndef _EMULATOR_INSTRUCTIONS_H
#define _EMULATOR_INSTRUCTIONS_H

#include <stdint.h>
#include <stdio.h>
#include "emu_core.h"

struct operand
{
    int type;
    union {
        uint16_t value; //immediate value
        uint16_t address; //memory address
        uint16_t reg; //register
    };

    uint16_t disp; //Only used for regindir
};

struct instruction
{
    uint8_t mnemonic;
    uint8_t condition;

    struct operand dst;
    struct operand src;
};

void emu_print_instr(FILE* dst, struct emulator_state* state, struct instruction* instr);

int emu_fetch_next_instruction(struct emulator_state* state, struct instruction* instr);
int emu_is_valid_instruction(struct instruction* instr);

int emu_execute_instruction(struct emulator_state* state, struct instruction* instr);

typedef int (*instr_impl_func)(struct emulator_state* state, struct instruction* instr);

extern instr_impl_func instr_implementation[INSTRUCTION_SET_SIZE];

void emu_call_int(struct emulator_state* state, uint8_t entry);

#endif