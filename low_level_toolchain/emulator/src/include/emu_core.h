#ifndef _EMULATOR_CORE_H
#define _EMULATOR_CORE_H

#include <stdint.h>
#include "emu_constants.h"

struct cpu
{
    uint16_t registers[8];
    uint16_t psw;
};

struct emulator_state
{
    struct cpu cpu;
    uint8_t memory[MEMORY_SIZE];

    uint8_t timer_irq;
    uint8_t kbd_input_irq;

    uint8_t kbd_reg_read;

    uint8_t sig_stop;
};

uint16_t emu_mem_read_word_raw (struct emulator_state* state, uint16_t address);
void emu_mem_write_word_raw (struct emulator_state* state, uint16_t address, uint16_t word);

uint16_t emu_mem_read_word (struct emulator_state* state, uint16_t address);
void emu_mem_write_word (struct emulator_state* state, uint16_t address, uint16_t word);

void emu_mem_dump(struct emulator_state* state, const char* file_path);

int emu_handle_irq(struct emulator_state* state);

void emu_psw_set_zero(struct emulator_state* state, uint8_t val);
void emu_psw_set_overflow(struct emulator_state* state, uint8_t val);
void emu_psw_set_carry(struct emulator_state* state, uint8_t val);
void emu_psw_set_negative(struct emulator_state* state, uint8_t val);
uint8_t emu_psw_get_zero(struct emulator_state* state);
uint8_t emu_psw_get_overflow(struct emulator_state* state);
uint8_t emu_psw_get_carry(struct emulator_state* state);
uint8_t emu_psw_get_negative(struct emulator_state* state);
void emu_psw_set_irq_mask(struct emulator_state* state, uint8_t val);
uint8_t emu_psw_get_irq_mask(struct emulator_state* state);
void emu_psw_set_timer_irq_mask(struct emulator_state* state, uint8_t val);
uint8_t emu_psw_get_timer_irq_mask(struct emulator_state* state);

uint16_t emu_get_int_routine_address(uint8_t int_no);


#endif