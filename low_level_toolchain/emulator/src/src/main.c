#include <stdio.h>

#include "emu_api.h"
#include "emu_core.h"

int main(int argc, const char* argv[])
{
    struct emulator_state state;
    if(!emu_init(&state, argv + 1, argc - 1))
        return 1;
    emu_run(&state);

    return 0;
}