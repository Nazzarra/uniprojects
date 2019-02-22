#include "emu_core.h"
#include "emu_api.h"
#include "emu_instr.h"

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include <sys/select.h>
#include <sys/time.h>

#include "time.h"

#define TRACE 0

uint16_t emu_mem_read_word_raw(struct emulator_state* state, uint16_t address)
{
    int16_t result = 0;
    result = state->memory[address] | (state->memory[address + 1] << 8);
    
    return result;
}

void emu_mem_write_word_raw(struct emulator_state* state, uint16_t address, uint16_t word)
{
    state->memory[address] = word & 0xFF;
    word >>= 8;
    state->memory[address + 1] = word & 0xFF;
}

uint16_t emu_mem_read_word(struct emulator_state* state, uint16_t address)
{
    if(address == KBD_INPUT_REG)
        state->kbd_reg_read = 1;
    return emu_mem_read_word_raw(state, address);
}

void emu_mem_write_word(struct emulator_state* state, uint16_t address, uint16_t word)
{
    emu_mem_write_word_raw(state, address, word);
    if(address == TERM_OUTPUT_REG){
        if(word == 0x10)
            printf("\n");
        else
            printf("%c", (uint8_t)word);

        fflush(stdout);
    }
}

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

void emu_mem_dump(struct emulator_state* state, const char* file_path)
{
    FILE* out = fopen(file_path, "w");
    if(out == NULL){
        printf("Failed to open file %s for writing\n", file_path);
        exit(666);
    }

    char buffer[5];

    for(int i = 0; i < MEMORY_SIZE / 16; ++i){
        emu_val_to_hex(i * 16, 2, buffer);
        fprintf(out, "%04d %s", i * 16, buffer);
        
        for(int j = 0; j < 16; ++j){
            emu_val_to_hex(state->memory[i * 16 + j], 1, buffer);
            fprintf(out, " %s", buffer);
        }

        fprintf(out, "\n");
    }

    fclose(out);
}

static void emu_dump_state(struct emulator_state* state, FILE* dst)
{
    fprintf(dst, "Registers: ");
    char buffer[5];
    for(int i = 0; i < 8; ++i){
        emu_val_to_hex(state->cpu.registers[i], 2, buffer);
        fprintf(dst, "r%d : %s ", i, buffer);
    }
    fprintf(dst, "\n");

    emu_val_to_hex(state->cpu.psw, 2, buffer);
    fprintf(dst, "PSW: %s\n", buffer);

    fprintf(dst, "Flags: Z - %d N - %d O - %d C - %d\n", emu_psw_get_zero(state), emu_psw_get_negative(state), emu_psw_get_overflow(state), emu_psw_get_carry(state));
}

static void emu_dump_stack(struct emulator_state* state, FILE* dst)
{
    uint16_t* sp = &state->cpu.registers[REGISTER_SP];
    char buffer[5];
    uint16_t start_addr, end_addr;

    start_addr = *sp - (*sp & 0xF);
    end_addr = *sp | 0xF;
    if((*sp & 0xF) == 0xF){
        end_addr = *sp + 0x10;
    }

    end_addr++;

    for(uint16_t addr = start_addr; addr != end_addr; ++addr){
        if(addr % 16 == 0){
            emu_val_to_hex(addr, 2, buffer);
            fprintf(dst, "%s ", buffer);
        }

        emu_val_to_hex(state->memory[addr], 1, buffer);
        fprintf(dst, "%s ", buffer);
        if((addr & 0xF) == 0xF)
            fprintf(dst, "\n");
    }
}

uint16_t emu_get_int_routine_address(uint8_t int_no)
{
    return int_no * 2;
}

#ifdef TRACE
    FILE* trace;
#endif

struct termios prev_termios;

static void emu_init_terminal()
{
    struct termios tmp;

    if (tcgetattr(fileno(stdin), &tmp) == -1){
        printf("Failed to read terminal attributes.\n");
        exit(666);
    }

    prev_termios = tmp;
    tmp.c_lflag &= ~ECHO;
    tmp.c_lflag &= ~ICANON;    
    fflush(stdout);
    if (tcsetattr(fileno(stdin), TCSANOW, &tmp) == -1){
        printf("Failed to set terminal attributes.\n");
        exit(666);
    }
}

static void emu_restore_terminal()
{
    if (tcsetattr(fileno(stdin), TCSANOW, &prev_termios) == -1){
        printf("Failed to restore terminal.\n");
    }
}

/* Ty stack overflow */
int emu_wait_kbhit()
{
    int STDIN_FILENO = fileno(stdin);
    // timeout structure passed into select
    struct timeval tv;
    // fd_set passed into select
    fd_set fds;
    // Set up the timeout.
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // Zero out the fd_set - make sure it's pristine
    FD_ZERO(&fds);
    // Set the FD that we want to read
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    // select takes the last file descriptor value + 1 in the fdset to check,
    // the fdset for reads, writes, and errors.  We are only passing in reads.
    // the last parameter is the timeout.  select will return if an FD is ready or 
    // the timeout has occurred
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    // return 0 if STDIN is not ready to be read.
    return FD_ISSET(STDIN_FILENO, &fds);
}

#ifdef TRACE
#define TRACE_CNT_MAX 50
#endif

void emu_run(struct emulator_state* state)
{
    struct instruction instr;

    #ifdef TRACE
        trace = fopen("trace", "w");
        if(trace == NULL){
            printf("Failed to open trace file.\n");
            exit(666);
        }
    #endif

    emu_init_terminal();

    printf("Emulator ready! Press any character to start");
    getchar();
    system("clear");

    #ifdef TRACE
        emu_dump_state(state, trace);
        int traced = 0;
    #endif

    emu_call_int(state, INTERRUPT_ROUTINE_START);

    clock_t time_start = clock();

    while(!state->sig_stop){
    
        #ifdef TRACE
            if(traced < TRACE_CNT_MAX){
                fprintf(trace, "Traced[%d]\n", traced);
                emu_dump_stack(state, trace);
                fprintf(trace, "\n");
                ++traced;
            }
        #endif

        if(!emu_fetch_next_instruction(state, &instr)){
            emu_call_int(state, INTERRUPT_ROUTINE_ILLEGAL_INSTR);
            continue;
        }

        #ifdef TRACE
            if(traced < TRACE_CNT_MAX){
                emu_print_instr(trace, state, &instr);
            }
        #endif

        if(!emu_is_valid_instruction(&instr)){
            emu_call_int(state, INTERRUPT_ROUTINE_ILLEGAL_INSTR);
            continue;
        }

        if(!emu_execute_instruction(state, &instr)){
            emu_call_int(state, INTERRUPT_ROUTINE_ILLEGAL_INSTR);
            continue;
        }

        if(state->kbd_reg_read && emu_psw_get_irq_mask(state)){
            if(emu_wait_kbhit()){
                uint16_t ch = getchar();
                emu_mem_write_word_raw(state, KBD_INPUT_REG, ch);
                state->kbd_reg_read = 0;
                state->kbd_input_irq = 1;
            }
        }

        if(emu_psw_get_irq_mask(state)){
            if(state->kbd_input_irq){
                state->kbd_input_irq = 0;
                emu_call_int(state, INTERRUPT_ROUTINE_KBD);    
                #ifdef TRACE
                    traced = 0;
                #endif
            }
            else if(emu_psw_get_timer_irq_mask(state)){
                if(clock() - time_start >= CLOCKS_PER_SEC){
                    time_start = clock();
                    emu_call_int(state, INTERRUPT_ROUTINE_TIMER);
                }
            }
        }

        #ifdef TRACE
            if(traced < TRACE_CNT_MAX){
                emu_dump_state(state, trace);    
                fflush(trace);
            }
        #endif
    }

    #ifdef TRACE
        fclose(trace);
    #endif

    if(state->sig_stop)
        printf("\nProgram called INTR 7(EXIT)\n");
        
    emu_restore_terminal();
}