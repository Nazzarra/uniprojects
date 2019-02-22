#include "emu_api.h"
#include "emu_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct symtab_entry
{
    char symname[MAX_SYMBOL_NAME];
    uint32_t section;
    uint16_t value;
    uint8_t linkage; 
    uint8_t type;
    uint16_t size;
};

struct reltab_entry
{
    uint16_t offset;
    uint16_t type;
    uint16_t symbol;
};

struct symtab 
{
    struct symtab_entry* symtab;
    uint32_t symbol_cnt;
};

struct reltab
{
    struct reltab_entry* reldata;
    uint32_t reldata_cnt;
};

struct obj_file
{
    struct symtab symtab;
    struct reltab rel_tabs[SECTION_CNT - 2];
};

struct emu_init_context
{
    struct obj_file* obj_files;
    int obj_file_cnt, obj_file_curr;

    struct symtab symtab;

    struct emulator_state* state;
};

static int emu_load_file(struct emu_init_context* ctx, const char* file_path);
static int emu_match_symbols(struct emu_init_context* ctx);
static void emu_fix_relocations(struct emu_init_context* ctx);

static void emu_symtab_add_symbol(struct symtab* symtab, struct symtab_entry* entry)
{
    if(symtab->symbol_cnt == 0)
        symtab->symtab = malloc(sizeof(struct symtab_entry));
    else
        symtab->symtab = realloc(symtab->symtab, sizeof(struct symtab_entry) * (symtab->symbol_cnt + 1));
    
    struct symtab_entry* target = symtab->symtab + symtab->symbol_cnt;
    symtab->symbol_cnt++;

    target->linkage = entry->linkage;
    target->section = entry->section;
    target->size = entry->size;
    strcpy(target->symname, entry->symname);
    target->type = entry->type;
    target->value = entry->value;
}

static int emu_symtab_get_symbol(struct symtab* symtab, char* symname)
{
    for(int i = 0; i < symtab->symbol_cnt; ++i)
        if(strcmp(symname, symtab->symtab[i].symname) == 0)
            return i;
    
    return -1;
}

static int emu_symtab_get_section_symbol(struct symtab* symtab, int section)
{
    const char* sections[] = {".UND", ".TEXT", ".DATA", ".RODATA", ".BSS"};
    for(int i = 0; i < symtab->symbol_cnt; ++i){
        if(strcmp(symtab->symtab[i].symname, sections[section]) == 0)
            return i;
    }

    return -1;
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

static int emu_sections_overlap(struct symtab_entry* section_one, struct symtab_entry* section_two)
{
    int sec_a_start = section_one->value;
    int sec_a_end = section_one->value + section_one->size - 1;

    int sec_b_start = section_two->value;
    int sec_b_end = section_two->value + section_two->size - 1;

    if(sec_a_start <= sec_b_start && sec_a_end >= sec_b_start)
        return 1;

    if(sec_a_start >= sec_b_start && sec_a_end <= sec_b_end)
        return 1;

    if(sec_a_start <= sec_b_end && sec_a_end >= sec_b_end)
        return 1;

    return 0;
}

static void emu_create_init_ctx(struct emu_init_context* ctx, struct emulator_state* state, int file_count)
{
    ctx->obj_file_curr = 0;
    ctx->obj_file_cnt = file_count;
    ctx->obj_files = malloc(sizeof(struct obj_file) * ctx->obj_file_cnt);

    ctx->symtab.symbol_cnt = 0;
    ctx->state = state;

    for(int i = 0; i < MEMORY_SIZE; ++i)
        state->memory[i] = 0;
}

static void emu_free_init_ctx(struct emu_init_context* ctx)
{
    free(ctx->obj_files);
    free(ctx->symtab.symtab);
}

static void emu_init_state(struct emu_init_context* ctx)
{
    ctx->state->kbd_input_irq = 0;
    ctx->state->timer_irq = 0;
    ctx->state->cpu.psw = 0;

    int start_index = emu_symtab_get_symbol(&ctx->symtab, START_SYMBOL);
    int start_address = ctx->symtab.symtab[start_index].value;
    ctx->state->cpu.registers[REGISTER_PC] = start_address;
    ctx->state->cpu.registers[REGISTER_SP] = PER_MEM_START;

    ctx->state->sig_stop = 0;
    ctx->state->kbd_reg_read = 1;
}

int emu_init(struct emulator_state* state, const char* files[], int file_count)
{
    struct emu_init_context ctx;

    emu_create_init_ctx(&ctx, state, file_count);
    
    for(int i = 0; i < file_count; ++i)
        if(!emu_load_file(&ctx, files[i]))
            return 0;
    
    if(!emu_match_symbols(&ctx))
        return 0;

    emu_fix_relocations(&ctx);
    emu_mem_dump(ctx.state, "core_dump");

    emu_init_state(&ctx);
    
    emu_free_init_ctx(&ctx);
    return 1;
}

//Test more
static int emu_file_overlaps(struct emu_init_context* ctx, const struct symtab* symtab)
{
    for(int i = 0; i < symtab->symbol_cnt; ++i){
        if(symtab->symtab[i].type == SYMTYPE_SECTION && symtab->symtab[i].section != SECTION_UND){
            struct symtab_entry* section = &symtab->symtab[i];
            
            for(int j = 0; j < ctx->obj_file_curr; ++j){
                struct obj_file* file = ctx->obj_files + j;

                for(int k = 0; k < file->symtab.symbol_cnt; ++k){
                    if(file->symtab.symtab[k].type == SYMTYPE_SECTION && file->symtab.symtab[k].section != SECTION_UND){
                        struct symtab_entry* file_section = &file->symtab.symtab[k];

                        if(emu_sections_overlap(section, file_section))
                            return 1;
                    }
                }
            }
        }
    }

    return 0;
}

static int emu_load_file(struct emu_init_context* ctx, const char* file_path)
{
    FILE* in = fopen(file_path, "rb");

    fprintf(stderr, "Loading file %s\n", file_path);

    if(in == NULL){
        printf("Failed to open file %s for reading!\n", file_path);
        return 0;
    }

    struct obj_file* file = ctx->obj_files + ctx->obj_file_curr;
    int cnt;
    uint32_t symbol_cnt;
    struct symtab_entry* symbol_table = NULL;

    //Symbol table reading
    cnt = fread(&symbol_cnt, sizeof(uint32_t), 1, in);
    if(cnt != 1){
        printf("Invalid emulator file(failed to read symbol count)!\n");
        return 0;
    }

    symbol_table = malloc(sizeof(struct symtab_entry) * symbol_cnt);
    fprintf(stderr, "Symbol count: %d\n", symbol_cnt);

    cnt = fread(symbol_table, sizeof(struct symtab_entry), symbol_cnt, in);
    if(cnt != symbol_cnt){
        printf("Invalid emulator file(failed to read symbol table)");
        return 0;
    }

    file->symtab.symbol_cnt = symbol_cnt;
    file->symtab.symtab = symbol_table;

    const char* sections[] = {"UND", "TEXT", "DATA", "RODATA", "BSS"};
    const char linkage[] = { 'L', 'G' };
    
    fprintf(stderr, "Symbol table\n");
    for(int i = 0; i < symbol_cnt; ++i){
        fprintf(stderr, "%2d. %10s %5d %7s %c %5d %s\n", i, symbol_table[i].symname, symbol_table[i].value, sections[symbol_table[i].section], linkage[symbol_table[i].linkage], symbol_table[i].size, symbol_table[i].type == SYMTYPE_SYM ? "SYM" : "SECTION" );
    }

    if(emu_file_overlaps(ctx, &file->symtab)){
        printf("Sections of the input files overlap.\n");
        return 0;
    }

    //Section data reading
    for(int i = 0; i < symbol_cnt; ++i){
        if(symbol_table[i].type == SYMTYPE_SECTION && symbol_table[i].section != SECTION_UND && symbol_table[i].section != SECTION_BSS){
            fprintf(stderr, "Read section %s\n", sections[symbol_table[i].section]);
            uint16_t start_address = symbol_table[i].value;
            uint16_t size = symbol_table[i].size;
            cnt = fread(ctx->state->memory + start_address, sizeof(uint8_t), size, in);
            if(cnt != size){
                printf("Unable to read section %s data.\n", sections[symbol_table[i].section]);
                return 0;
            }
            fprintf(stderr, "Section %s\n", symbol_table[i].symname);
            char buff[3] = {0, 0, 0};
            for(int j = 0; j < size; ++j){
                emu_val_to_hex(ctx->state->memory[start_address + j], 1, buff);
                fprintf(stderr, "%s ", buff);
                if((j + 1) % 16 == 0 && j + 1 != size)
                    fprintf(stderr, "\n");
            }
            fprintf(stderr, "\n");
        }
    }

    uint32_t reldata_cnt;
    struct reltab_entry* reldata;
    //Rellocation data reading
    for(int i = 0; i < symbol_cnt; ++i){
        if(symbol_table[i].type == SYMTYPE_SECTION && symbol_table[i].section != SECTION_UND && symbol_table[i].section != SECTION_BSS){
            cnt = fread(&reldata_cnt, sizeof(uint32_t), 1, in);
            if(cnt != 1){
                printf("Failed to read section %s reltab entry count.\n", sections[symbol_table[i].section]);
                return 0;
            }

            reldata = malloc(sizeof(struct reltab_entry) * reldata_cnt);
            cnt = fread(reldata, sizeof(struct reltab_entry), reldata_cnt, in);
            if(cnt != reldata_cnt){
                printf("Failed to read section %s reltab entries.\n", sections[symbol_table[i].section]);
                return 0;
            }

            fprintf(stderr, "Relocation table - section %s\n", sections[symbol_table[i].section]);

            file->rel_tabs[symbol_table[i].section - 1].reldata_cnt = reldata_cnt;
            file->rel_tabs[symbol_table[i].section - 1].reldata = reldata;

            const char* alloc_type[] = {"REL", "ABS"};
            char addr_buff[5];

            for(int j = 0; j < reldata_cnt; ++j){
                emu_val_to_hex(reldata[j].offset, 2, addr_buff);
                fprintf(stderr, " %4s %4s %3d\n", addr_buff, alloc_type[reldata[j].type], reldata[j].symbol);
            }
        }
    }
    
    fprintf(stderr, "\n");
    fclose(in);
    ctx->obj_file_curr++;
    return 1;
}

static int emu_match_symbols(struct emu_init_context* ctx)
{
    for(int i = 0; i < ctx->obj_file_cnt; ++i){
        struct symtab* symtab = &ctx->obj_files[i].symtab;
        for(int j = 0; j < symtab->symbol_cnt; ++j){
            if(symtab->symtab[j].linkage == LINKAGE_GLOBAL){
                int index = emu_symtab_get_symbol(&ctx->symtab, symtab->symtab[j].symname);
                
                if(index != -1){
                    struct symtab_entry* entry = ctx->symtab.symtab + index;
                    struct symtab_entry* symbol = symtab->symtab + j;

                    if(entry->section == SECTION_UND && symbol->section != SECTION_UND){
                        entry->section = symbol->section;
                        int section_value = symtab->symtab[emu_symtab_get_section_symbol(symtab, symbol->section)].value;
                        entry->value = symbol->value + section_value;
                    }
                    else if(entry->section != SECTION_UND && symbol->section != SECTION_UND){
                        printf("Multiple definitions of symbol %s\n", entry->symname);
                        return 0;
                    }
                }
                else{
                    emu_symtab_add_symbol(&ctx->symtab, symtab->symtab + j);
                    if(symtab->symtab[j].section != SECTION_UND){
                        int section_value = symtab->symtab[emu_symtab_get_section_symbol(symtab, symtab->symtab[j].section)].value;
                        ctx->symtab.symtab[ctx->symtab.symbol_cnt - 1].value += section_value;
                    }
                }
            }
        }
    }


    const char* sections[] = {"UND", "TEXT", "DATA", "RODATA", "BSS"};
    const char linkage[] = { 'L', 'G' };
    struct symtab_entry* symbol_table = ctx->symtab.symtab;

    for(int i = 0; i < ctx->symtab.symbol_cnt; ++i){
        if(ctx->symtab.symtab[i].section == SECTION_UND){
            printf("Unresolved global symbol %s\n", ctx->symtab.symtab[i].symname);
            return 0;
        }
        
        fprintf(stderr, "%2d. %10s %5d %7s %c %5d %s\n", i, symbol_table[i].symname, symbol_table[i].value, sections[symbol_table[i].section], linkage[symbol_table[i].linkage], symbol_table[i].size, symbol_table[i].type == SYMTYPE_SYM ? "SYM" : "SECTION" );
    }

    int start_symbol = emu_symtab_get_symbol(&ctx->symtab, START_SYMBOL);
    if(start_symbol == -1){
        printf("Unresolved global symbol START - Unable to determine program start address.");
        return 0;
    }

    return 1;
}


static void emu_fix_relocation(struct emu_init_context* ctx, int section, struct reltab_entry* entry, struct symtab* symtab)
{
    struct symtab_entry* symbol = symtab->symtab + entry->symbol;
    struct symtab_entry* section_entry = symtab->symtab + emu_symtab_get_section_symbol(symtab, section);

    uint16_t offset = entry->offset + section_entry->value;

    if(entry->type == RELOCATION_ABSOLUTE){
        if(symbol->linkage == LINKAGE_GLOBAL){
            int symbol_value = ctx->symtab.symtab[emu_symtab_get_symbol(&ctx->symtab, symbol->symname)].value;
            emu_mem_write_word_raw(ctx->state, offset, symbol_value);
        }
        else{
            uint16_t val = emu_mem_read_word_raw(ctx->state, offset);
            val += symbol->value;
            emu_mem_write_word_raw(ctx->state, offset, val);
        }
    }
    //Relative
    else{
        int symbol_value;
        if(symbol->linkage == LINKAGE_GLOBAL)
            symbol_value = ctx->symtab.symtab[emu_symtab_get_symbol(&ctx->symtab, symbol->symname)].value;
        else
            symbol_value = symbol->value;

        uint16_t val = emu_mem_read_word_raw(ctx->state, offset);
        val = symbol_value + val - offset;
        emu_mem_write_word_raw(ctx->state, offset, val);
    }
}

static void emu_fix_relocations(struct emu_init_context* ctx)
{
    for(int i = 0; i < ctx->obj_file_cnt; ++i){
        struct obj_file* file = ctx->obj_files + i;
        for(int section = 0; section < SECTION_CNT - 2; ++section){
            struct reltab* reltab = file->rel_tabs + section;
            for(int k = 0; k < reltab->reldata_cnt; ++k){
                struct reltab_entry* entry = reltab->reldata + k;
                emu_fix_relocation(ctx, section + 1, entry, &file->symtab);
            }
        }
    }
}