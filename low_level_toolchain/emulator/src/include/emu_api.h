#ifndef _EMULATOR_API_H
#define _EMULATOR_API_H

struct emulator_state;

int emu_init(struct emulator_state* state, const char* files[], int file_count);

void emu_run(struct emulator_state* state);

#endif