#include "emu_instr.h"

#include <stdio.h>
#include <stdlib.h>

uint32_t emu_fetch_operand(struct emulator_state* state, struct operand* operand)
{
    if(operand->type == OPERAND_IMM)
        return operand->value;
    if(operand->type == OPERAND_PSW)
        return state->cpu.psw;
    if(operand->type == OPERAND_MEMDIR)
        return emu_mem_read_word(state, operand->address);
    if(operand->type == OPERAND_REGDIR)
        return state->cpu.registers[operand->reg];
    if(operand->type == OPERAND_REGINDIR)
        return emu_mem_read_word(state, state->cpu.registers[operand->reg] + operand->disp);

    printf("Fatal error: unidentified addressing mode");
    exit(666);
    return 0;
}

static uint8_t emu_get_bit(uint16_t val, uint8_t bit)
{
    return (val >> bit) & 0x1;
}

void emu_set_bit(uint16_t* val, uint8_t bit, uint8_t bit_val)
{
    uint16_t mask = ~(1U << bit);
    *val = (*val & mask) | (bit_val << bit);
}

void emu_psw_set_irq_mask(struct emulator_state* state, uint8_t val)
{
    emu_set_bit(&state->cpu.psw, PSW_BIT_GLOBAL_IRQ_MASK, val);
}

uint8_t emu_psw_get_irq_mask(struct emulator_state* state)
{
    return emu_get_bit(state->cpu.psw, PSW_BIT_GLOBAL_IRQ_MASK);
}

void emu_psw_set_timer_irq_mask(struct emulator_state* state, uint8_t val)
{
    emu_set_bit(&state->cpu.psw, PSW_BIT_TIMER_IRQ_MASK, val);
}

uint8_t emu_psw_get_timer_irq_mask(struct emulator_state* state)
{
    return emu_get_bit(state->cpu.psw, PSW_BIT_TIMER_IRQ_MASK);
}


void emu_psw_set_zero(struct emulator_state* state, uint8_t val)
{
    emu_set_bit(&state->cpu.psw, PSW_BIT_ZERO, val);
}

void emu_psw_set_overflow(struct emulator_state* state, uint8_t val)
{
    emu_set_bit(&state->cpu.psw, PSW_BIT_OVERFLOW, val);
}


void emu_psw_set_carry(struct emulator_state* state, uint8_t val)
{
    emu_set_bit(&state->cpu.psw, PSW_BIT_CARRY, val);
}

void emu_psw_set_negative(struct emulator_state* state, uint8_t val)
{
    emu_set_bit(&state->cpu.psw, PSW_BIT_NEGATIVE, val);
}

uint8_t emu_psw_get_zero(struct emulator_state* state)
{
    return emu_get_bit(state->cpu.psw, PSW_BIT_ZERO);
}

uint8_t emu_psw_get_overflow(struct emulator_state* state)
{
    return emu_get_bit(state->cpu.psw, PSW_BIT_OVERFLOW);
}

uint8_t emu_psw_get_carry(struct emulator_state* state)
{
    return emu_get_bit(state->cpu.psw, PSW_BIT_CARRY);
}

uint8_t emu_psw_get_negative(struct emulator_state* state)
{
    return emu_get_bit(state->cpu.psw, PSW_BIT_NEGATIVE);
}

static int emu_get_sign(uint32_t operand)
{
    return (operand & 0x8000) > 0;
}

static int emu_get_carry(uint32_t value)
{
    return (value & 0x10000) > 0;
}

static void emu_save_to_dst(struct emulator_state* state, struct operand* operand, uint16_t value)
{
    if(operand->type == OPERAND_PSW)
        state->cpu.psw = value;
    else if(operand->type == OPERAND_MEMDIR)
        emu_mem_write_word(state, operand->address, value);
    else if(operand->type == OPERAND_REGDIR)
        state->cpu.registers[operand->reg] = value;
    else if(operand->type == OPERAND_REGINDIR)
        emu_mem_write_word(state, state->cpu.registers[operand->reg] + operand->disp, value);
}

static int emu_add(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = dst_val + src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, final_result == 0);
    emu_psw_set_negative(state, final_result >= NEG_NUM_RANGE_START);
    emu_psw_set_overflow(state, (emu_get_sign(dst_val) == emu_get_sign(src_val)) && (emu_get_sign(result) != emu_get_sign(dst_val)));
    emu_psw_set_carry(state, emu_get_carry(result));
    
    emu_save_to_dst(state, &instr->dst, final_result);
    return 1;
}

static int emu_sub(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = dst_val - src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, final_result == 0);
    emu_psw_set_negative(state, final_result >= NEG_NUM_RANGE_START);
    emu_psw_set_overflow(state, 0);
    if(emu_get_sign(dst_val) == 0 && emu_get_sign(src_val) == 1 && emu_get_sign(result) == 1)
        emu_psw_set_overflow(state, 1);
    else if(emu_get_sign(dst_val) == 1 && emu_get_sign(src_val) == 0 && emu_get_sign(result) == 0)
        emu_psw_set_overflow(state, 1);

    emu_psw_set_carry(state, emu_get_carry(result));

    emu_save_to_dst(state, &instr->dst, final_result);
    return 1; 
}

static int emu_mul(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint16_t result = (int16_t)dst_val * (int16_t)src_val;
    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, result >= NEG_NUM_RANGE_START);

    emu_save_to_dst(state, &instr->dst, result);
    return 1;
}

static int emu_div(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    if(src_val == 0)
        return 0;

    uint16_t result = (int16_t)dst_val / (int16_t)src_val;
    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, result >= NEG_NUM_RANGE_START);


    emu_save_to_dst(state, &instr->dst, result);
    return 1;
}

static int emu_cmp(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = dst_val - src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, final_result >= NEG_NUM_RANGE_START);
    emu_psw_set_overflow(state, 0);
    if(emu_get_sign(dst_val) == 0 && emu_get_sign(src_val) == 1 && emu_get_sign(result) == 1)
        emu_psw_set_overflow(state, 1);
    else if(emu_get_sign(dst_val) == 1 && emu_get_sign(src_val) == 0 && emu_get_sign(result) == 0)
        emu_psw_set_overflow(state, 1);

    emu_psw_set_carry(state, emu_get_carry(result));
    return 1;
}

static int emu_and(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = dst_val & src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, result >= NEG_NUM_RANGE_START);

    emu_save_to_dst(state, &instr->dst, final_result);
    return 1;
}

static int emu_or(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = dst_val | src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, result >= NEG_NUM_RANGE_START);

    emu_save_to_dst(state, &instr->dst, final_result);
    return 1;
}

static int emu_not(struct emulator_state* state, struct instruction* instr)
{
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = ~src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, final_result == 0);
    emu_psw_set_negative(state, final_result >= NEG_NUM_RANGE_START);

    emu_save_to_dst(state, &instr->dst, final_result);
    return 1;
}

static int emu_test(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    uint32_t src_val = emu_fetch_operand(state, &instr->src);

    uint32_t result = dst_val & src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, final_result == 0);
    emu_psw_set_negative(state, final_result >= NEG_NUM_RANGE_START);

    return 1;
}

static void emu_push_skeleton(struct emulator_state* state, uint16_t val)
{
    uint16_t* sp = &state->cpu.registers[REGISTER_SP];
    *sp -= 2;
    emu_mem_write_word(state, *sp, val);
}

static int emu_push(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    
    emu_push_skeleton(state, dst_val);
    return 1;
}

static uint16_t emu_pop_skeleton(struct emulator_state* state)
{
    uint16_t* sp = &state->cpu.registers[REGISTER_SP];
    uint16_t data = emu_mem_read_word(state, *sp);

    *sp += 2;
    return data;
}

static int emu_pop(struct emulator_state* state, struct instruction* instr)
{
    uint16_t data = emu_pop_skeleton(state);

    emu_save_to_dst(state, &instr->dst, data);
    return 1;
}

static int emu_call(struct emulator_state* state, struct instruction* instr)
{
    uint16_t dst_val = (uint16_t)emu_fetch_operand(state, &instr->dst);
    uint16_t* pc = &state->cpu.registers[REGISTER_PC];

    uint16_t kill_routine = emu_get_int_routine_address(INTERRUPT_ROUTINE_KILL);
    if((instr->dst.type == OPERAND_MEMDIR && instr->dst.address == kill_routine) || (instr->dst.type == OPERAND_REGINDIR && state->cpu.registers[instr->dst.reg] + instr->dst.disp == kill_routine)){
        state->sig_stop = 1;
        return 1;
    }

    emu_push_skeleton(state, *pc);
    *pc = dst_val;
    return 1;
}

static int emu_iret(struct emulator_state* state, struct instruction* instr)
{
    uint16_t pc = emu_pop_skeleton(state);
    uint16_t psw = emu_pop_skeleton(state);

    state->cpu.psw = psw;
    state->cpu.registers[REGISTER_PC] = pc;
    return 1;
}

static int emu_mov(struct emulator_state* state, struct instruction* instr)
{
    uint16_t result = (uint16_t)emu_fetch_operand(state, &instr->src);

    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, result >= NEG_NUM_RANGE_START);

    emu_save_to_dst(state, &instr->dst, result);
    return 1;
}

static int emu_shl(struct emulator_state* state, struct instruction* instr)
{
    uint32_t dst_val = emu_fetch_operand(state, &instr->dst);
    int16_t src_val = (int16_t)emu_fetch_operand(state, &instr->src);
    if(src_val >= 16 || src_val < 0)
        return 0;

    uint32_t result = dst_val << src_val;
    uint16_t final_result = (uint16_t)result;

    emu_psw_set_zero(state, final_result == 0);
    emu_psw_set_negative(state, final_result >= NEG_NUM_RANGE_START);
    emu_psw_set_carry(state, ((result >> 16) & 0xFFFF) > 0);

    emu_save_to_dst(state, &instr->dst, final_result);
    return 1;
}

static int emu_shr(struct emulator_state* state, struct instruction* instr)
{
    int16_t dst_val = (int16_t)emu_fetch_operand(state, &instr->dst);
    int16_t src_val = (int16_t)emu_fetch_operand(state, &instr->src);

    if(src_val >= 16 || src_val < 0)
        return 0;

    uint16_t result = (uint16_t)(dst_val >> src_val);
    
    emu_psw_set_zero(state, result == 0);
    emu_psw_set_negative(state, result >= NEG_NUM_RANGE_START);
    emu_psw_set_carry(state, (dst_val - (result << src_val))  != 0);

    emu_save_to_dst(state, &instr->dst, result);
    return 1;
}

instr_impl_func instr_implementation[INSTRUCTION_SET_SIZE] = {emu_add, emu_sub, emu_mul, emu_div, emu_cmp, emu_and, emu_or, emu_not, emu_test, emu_push, emu_pop, emu_call, emu_iret, emu_mov, emu_shl, emu_shr };

void emu_call_int(struct emulator_state* state, uint8_t entry)
{
    uint16_t addr = emu_get_int_routine_address(entry);
    emu_push_skeleton(state, state->cpu.psw);
    emu_push_skeleton(state, state->cpu.registers[REGISTER_PC]);
    //Turn off interrupt nesting
    emu_psw_set_irq_mask(state, 0);
    state->cpu.registers[REGISTER_PC] = emu_mem_read_word(state, addr);
}
