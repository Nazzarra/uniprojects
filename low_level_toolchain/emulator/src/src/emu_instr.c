#include "emu_instr.h"

#include <stdio.h>
#include <stdlib.h>

const uint8_t instr_op_cnt[INSTRUCTION_SET_SIZE] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 0, 2, 2, 2};

static void emu_val_to_hex(uint32_t val, uint8_t bytes, char* output)
{
    static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    for(int i = bytes - 1; i >= 0; --i){
        output[i * 2 + 1] = hex_digits[val & 0xF];
        val >>= 4;
        output[i * 2] = hex_digits[val & 0xf];
        val >>= 4;
    }
    output[bytes * 2] = '\0'; 
}

static void emu_decode_operand(struct operand* operand, uint16_t code)
{
    int type = code >> 3;
    int val = code & 0x7;
    if(type == 0){
        if(val == 7)
            operand->type = OPERAND_PSW;
        else
            operand->type = OPERAND_IMM;
    }
    else if(type == 1){
        operand->type = OPERAND_REGDIR;
        operand->reg = val;
    }
    else if(type == 2){
        operand->type = OPERAND_MEMDIR;
    }
    else if(type == 3){
        operand->type = OPERAND_REGINDIR;
        operand->reg = val;
    }
}

static void emu_operand_read(struct operand* operand, uint16_t val)
{
    if(operand->type == OPERAND_IMM)
        operand->value = val;
    else if(operand->type == OPERAND_MEMDIR)
        operand->address = val;
    else if(operand->type == OPERAND_REGINDIR)
        operand->disp = val;
}

static int emu_operand_extra_size(struct operand* operand)
{
    if(operand->type == OPERAND_REGDIR || operand->type == OPERAND_PSW)
        return 0;

    return 1;
}

static void emu_decode_mnemonic(struct instruction* instr, uint16_t code)
{
    instr->mnemonic = (code >> 10) & 0xF;
}

static void emu_decode_condition(struct instruction* instr, uint16_t code)
{
    instr->condition = code >> 14;
}

static uint16_t emu_extract_first_operand(uint16_t code)
{
    return (code >> 5) & 0x1F;
}

static uint16_t emu_extract_second_operand(uint16_t code)
{
    return code & 0x1F;
}

uint32_t emu_fetch_operand(struct emulator_state* state, struct operand* operand);

static void emu_print_operand(FILE* dst, struct emulator_state* state, struct operand* operand)
{
    const char* types[] = { "Immediate", "PSW", "REGDIR", "MEMDIR", "REGINDIR" };
    char buffer[5];
    if(operand->type >= 0 && operand->type <= 5){
        fprintf(dst, "%s ", types[operand->type]);
        uint16_t val = emu_fetch_operand(state, operand);    
        if(operand->type == OPERAND_IMM){
            emu_val_to_hex(operand->value, 2, buffer);
            fprintf(dst, "%s\n", buffer);
        }
        else if(operand->type == OPERAND_PSW){
            fprintf(dst, "\n");
        }
        else if(operand->type == OPERAND_REGDIR){
            emu_val_to_hex(val, 2, buffer);
            fprintf(dst, "%d %s\n", operand->reg, buffer);
        }
        else if(operand->type == OPERAND_MEMDIR){
            emu_val_to_hex(operand->address, 2, buffer);
            fprintf(dst, "%s ", buffer);
            emu_val_to_hex(val, 2, buffer);
            fprintf(dst, "%s\n", buffer);
        }
        else if(operand->type == OPERAND_REGINDIR){
            emu_val_to_hex(operand->disp, 2, buffer);
            fprintf(dst, "%d %s ", operand->reg, buffer);
            emu_val_to_hex(val, 2, buffer);
            fprintf(dst, "%s\n", buffer);
        }
    }
    else
        fprintf(dst, "\n");
}

void emu_print_instr(FILE* dst, struct emulator_state* state, struct instruction* instr)
{
    const char* mnemonics[INSTRUCTION_SET_SIZE] = { "add", "sub", "mul", "div", "cmp", "and", "or", "not", "test", "push", "pop", "call", "iret", "mov", "shl", "shr" };
    fprintf(dst, "Instruction: %s\n", mnemonics[instr->mnemonic]);
    if(instr_op_cnt[instr->mnemonic] > 0){
        fprintf(dst, "Dst: ");
        emu_print_operand(dst, state, &instr->dst);
    }
    if(instr_op_cnt[instr->mnemonic] > 1){
        fprintf(dst, "Src: ");
        emu_print_operand(dst, state, &instr->src);
    }
}

int emu_fetch_next_instruction(struct emulator_state* state, struct instruction* instr)
{
    char buffer[5];
    struct cpu* cpu = &state->cpu;
    uint16_t* pc = (uint16_t*)&cpu->registers[REGISTER_PC];
    
    emu_val_to_hex(*pc, 2, buffer);
    uint16_t word_high = state->memory[*pc];
    uint16_t word_low = state->memory[*pc + 1];
    
    uint16_t code = (word_high << 8) | word_low;
    
    *pc += 2;
    
    emu_decode_mnemonic(instr, code);
    emu_decode_condition(instr, code);
    uint16_t first_op_code = emu_extract_first_operand(code);
    uint16_t second_op_code = emu_extract_second_operand(code);

    int extra_size = 0;
    if(instr_op_cnt[instr->mnemonic] > 0){
        emu_decode_operand(&instr->dst, first_op_code);
        extra_size = emu_operand_extra_size(&instr->dst);
        if(extra_size){
            uint16_t value = emu_mem_read_word(state, *pc);
            *pc += 2;
            emu_operand_read(&instr->dst, value);
        }
    }

    if(instr_op_cnt[instr->mnemonic] > 1){
        emu_decode_operand(&instr->src, second_op_code);
        if(emu_operand_extra_size(&instr->src)){
            if(extra_size){
                //Illegal instruction
                *pc -= 4;
                return 0;
            }

            uint16_t value = emu_mem_read_word(state, *pc);
            *pc += 2;
            emu_operand_read(&instr->src, value);
        }
    }

    return 1;
}

int emu_is_valid_instruction(struct instruction* instr)
{
    if(instr->dst.type == OPERAND_IMM)
        if(instr->mnemonic != MNEMONIC_CALL && instr->mnemonic != MNEMONIC_PUSH && instr->mnemonic != MNEMONIC_CMP && instr->mnemonic != MNEMONIC_TEST)
            return 0;

    return 1;
}

static int emu_instr_cond_ok(struct emulator_state* state, struct instruction* instr)
{
    if(instr->condition == INSTR_COND_NONE){
        return 1;
    }
    else if(instr->condition == INSTR_COND_EQUAL && emu_psw_get_zero(state) == 1){
        return 1;
    }
    else if(instr->condition == INSTR_COND_NOT_EQUAL && emu_psw_get_zero(state) == 0){
        return 1;
    }
    else if(instr->condition == INSTR_COND_GREATER_THAN){
        if(emu_psw_get_zero(state) == 1)
            return 0;
        
        if(emu_psw_get_negative(state) == emu_psw_get_overflow(state))
            return 1;
    }

    return 0;
}

int emu_execute_instruction(struct emulator_state* state, struct instruction* instr)
{
    if(!emu_instr_cond_ok(state, instr))
        return 1;

    if(!instr_implementation[instr->mnemonic](state, instr))
        return 0;

    return 1;
}