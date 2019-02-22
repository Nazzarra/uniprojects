#include "asm_api.h"
#include "asm_table_decl.h"
#include "asm_globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO FIX RELOCATIONS

#define INSTR_OK 0
#define INSTR_INVALID_OP_CNT 1
#define INSTR_INVALID_ADDRESS_MODES 2
#define INSTR_IMMEDIATE_AS_DST 3

static int asm_symtab_add_symbol(const char* symname, uint32_t section, uint16_t value, uint8_t linkage, uint8_t type)
{
    if(type == SYMTYPE_SECTION && linkage == LINKAGE_GLOBAL){
        printf("Unrecoverable symbol addition error.");
        exit(666);
    }

    if(symbol_cnt == symbol_table_size){
        symbol_table_size += 20;
        symbol_table = realloc(symbol_table, sizeof(struct symtab) * symbol_table_size);
        if(symbol_table == NULL){
            printf("Unrecoverable memory allocation error.");
            exit(666);
        }
    }

    strcpy(symbol_table[symbol_cnt].symname, symname);
    symbol_table[symbol_cnt].value = value;
    symbol_table[symbol_cnt].section = section;
    symbol_table[symbol_cnt].linkage = linkage;
    symbol_table[symbol_cnt].type = type;
    symbol_table[symbol_cnt].size = 0;

    return symbol_cnt++;
}

static int asm_symtab_get_symbol_entry(const char* symname)
{
    for(int i = 0; i < symbol_cnt; ++i)
        if(strcmp(symname, symbol_table[i].symname) == 0)
            return i;
    
    return -1;
}

static int asm_symtab_symbol_entry_exists(const char* symname)
{
    return asm_symtab_get_symbol_entry(symname) != -1;
}

static int asm_symtab_get_section_entry(int section)
{
    for(int i = 0; i < symbol_cnt; ++i)
        if(strcmp(sections[section], symbol_table[i].symname) == 0)
            return i;

    printf("Fatal error: cannot find target section");
    exit(666);
    return -1;
}

void asm_reltab_add_relocation(uint16_t section, uint16_t offset, uint16_t type, uint16_t symbol)
{
    section -= 1; //SECTION_TEXT = 1
    struct reltab* reltab = &section_reltab[section];
    if(reltab->cnt == reltab->size){
        reltab->size += 20;
        reltab->reldata = realloc(reltab->reldata, sizeof(struct reltab_entry) * reltab->size);
    }

    reltab->reldata[reltab->cnt].offset = offset;
    reltab->reldata[reltab->cnt].type = type;
    reltab->reldata[reltab->cnt].symbol = symbol;
    reltab->cnt++;
}

static uint8_t asm_instruction_is_valid(struct instruction_info* instr_info)
{
    //Invalid operand count
    if(instr_info->arg_cnt != instr_op_cnt[instr_info->mnemonic_id]){
        return INSTR_INVALID_OP_CNT;
    }
    //Wrong address modes
    if(instr_info->arg_cnt == 2 && instr_info->first_op.type != OPERAND_REGDIR && instr_info->second_op.type != OPERAND_REGDIR)
        return INSTR_INVALID_ADDRESS_MODES;

    //Destination is immediate operand
    if(instr_info->mnemonic_id == MNEMONIC_POP){    
        if(instr_info->first_op.type == OPERAND_IMM)
            return INSTR_IMMEDIATE_AS_DST;
    }
    else if(instr_info->arg_cnt == 2 && instr_info->mnemonic_id != MNEMONIC_CMP && instr_info->mnemonic_id != MNEMONIC_TEST){
        const struct operand_info* operand;
        if(operand_dst_first)
            operand = &instr_info->first_op;
        else
            operand = &instr_info->second_op;

        if(operand->type == OPERAND_IMM)
            return INSTR_IMMEDIATE_AS_DST;
    }

    return INSTR_OK;
}

static uint8_t asm_instruction_get_size(struct instruction_info* instr_info)
{
    if(instr_info->arg_cnt == 0)
        return INSTR_BASE_SIZE_BYTES;

    if(instr_info->arg_cnt == 1){
        if(instr_info->first_op.type == OPERAND_REGDIR)
            return INSTR_BASE_SIZE_BYTES;
        else
            return INSTR_EXTRA_SIZE_BYTES;
    }

    if(instr_info->first_op.type == OPERAND_REGDIR && instr_info->second_op.type == OPERAND_REGDIR)
        return INSTR_BASE_SIZE_BYTES;
    
    return INSTR_EXTRA_SIZE_BYTES;
}

int asm_fp_label(char* label_name)
{
    if(asm_symtab_symbol_entry_exists(label_name)){
        int symbol = asm_symtab_get_symbol_entry(label_name);
        //Global symbol
        if(symbol_table[symbol].section == SECTION_UND){
            symbol_table[symbol].section = current_section;
            symbol_table[symbol].value = section_loc;
            return 1;
        }
        sprintf(error_msg_buffer, "Symbol redefined: %s", label_name);
        return 0;
    }

    if(current_section == SECTION_UND){
        sprintf(error_msg_buffer, "Symbol %s declared outside of a section", label_name);
        return 0;
    }

    asm_symtab_add_symbol(label_name, current_section, section_loc, LINKAGE_LOCAL, SYMTYPE_SYM);
    return 1;
}

#define DIRECTIVE_OK 0
#define DIRECTIVE_INVALID_ARG_CNT 1
#define DIRECTIVE_INVALID_ARG_TYPE 2

static int asm_directive_get_arg_cnt(struct directive_info* dir_info)
{
    int cnt = 0;
    struct directive_arg* arg = dir_info->args;
    while(arg != NULL){
        ++cnt;
        arg = arg->next;
    }
    return cnt;
}

static int asm_directive_is_valid(struct directive_info* dir_info)
{
    //Check if valid argument count
    //.text, .data., .rodata, .bss have 0 args
    if(dir_info->directive_id == DIRECTIVE_TEXT || dir_info->directive_id == DIRECTIVE_DATA || dir_info->directive_id == DIRECTIVE_RODATA || dir_info->directive_id == DIRECTIVE_BSS){
        if(dir_info->args != NULL)
            return DIRECTIVE_INVALID_ARG_CNT;
    }
    //.align and .skip should have a single, numerical argument
    else if(dir_info->directive_id == DIRECTIVE_ALIGN || dir_info->directive_id == DIRECTIVE_SKIP){
        if(dir_info->args == NULL || dir_info->args->next != NULL)
            return DIRECTIVE_INVALID_ARG_CNT;

        if(dir_info->args->type != DIRECTIVE_ARG_NUMBER)
            return DIRECTIVE_INVALID_ARG_TYPE;
    }
    //.char, .word, .long, .global must not have 0 args
    else {
        if(dir_info->args == NULL)
            return DIRECTIVE_INVALID_ARG_CNT;
    }

    //Global directive should only have IDENTIFIER args
    if(dir_info->directive_id == DIRECTIVE_GLOBAL){
        struct directive_arg* arg = dir_info->args;
        while(arg != NULL){
            if(arg->type != DIRECTIVE_ARG_SYMBOL)
                return DIRECTIVE_INVALID_ARG_TYPE;

            arg = arg->next;
        }
    }

    return DIRECTIVE_OK;
}

static int asm_directive_get_size(struct directive_info* dir_info)
{
    //.text, .data., .rodata, .bss, .global have no size
    if(dir_info->directive_id == DIRECTIVE_TEXT || dir_info->directive_id == DIRECTIVE_DATA || dir_info->directive_id == DIRECTIVE_RODATA || dir_info->directive_id == DIRECTIVE_BSS || dir_info->directive_id == DIRECTIVE_GLOBAL)
        return 0;
    
    //.skip has the size of the argument
    if(dir_info->directive_id == DIRECTIVE_SKIP)
        return dir_info->args->num;
    
    //.align has the size required to reach alignment
    if(dir_info->directive_id == DIRECTIVE_ALIGN){
        if(section_loc % dir_info->args->num == 0)
            return 0;
        else
            return dir_info->args->num - section_loc % dir_info->args->num;
    }

    if(dir_info->directive_id == DIRECTIVE_CHAR)
        return asm_directive_get_arg_cnt(dir_info);
    if(dir_info->directive_id == DIRECTIVE_WORD)
        return asm_directive_get_arg_cnt(dir_info) * 2;
    if(dir_info->directive_id == DIRECTIVE_LONG)
        return asm_directive_get_arg_cnt(dir_info) * 4;

    printf("Unrecoverable directive size error.");
    exit(666);
}

static void asm_fp_directive_global(struct directive_info* dir_info)
{
    struct directive_arg* dir_arg = dir_info->args;
    while(dir_arg != NULL){
        if(asm_symtab_symbol_entry_exists(dir_arg->symname)){
            int symbol = asm_symtab_get_symbol_entry(dir_arg->symname);
            symbol_table[symbol].linkage = LINKAGE_GLOBAL;
        }
        else{
            asm_symtab_add_symbol(dir_arg->symname, SECTION_UND, 0, LINKAGE_GLOBAL, SYMTYPE_SYM);
        }
        dir_arg = dir_arg->next;
    }
}

//TODO add .end
int asm_fp_directive(struct directive_info* dir_info)
{
    int dir_error = asm_directive_is_valid(dir_info);
    uint32_t last_section;
    if(dir_error == DIRECTIVE_INVALID_ARG_CNT){
        sprintf(error_msg_buffer, "Directive has invalid argument count");
        return 0;
    }
    if(dir_error == DIRECTIVE_INVALID_ARG_TYPE){
        sprintf(error_msg_buffer, "Directive has invalid argument type");
        return 0;
    }

    if(dir_info->directive_id == DIRECTIVE_TEXT){
        last_section = current_section;
        current_section = SECTION_TEXT;
    }
    else if(dir_info->directive_id == DIRECTIVE_DATA){
        last_section = current_section;
        current_section = SECTION_DATA;
    }
    else if(dir_info->directive_id == DIRECTIVE_RODATA){
        last_section = current_section;
        current_section = SECTION_RODATA;
    }
    else if(dir_info->directive_id == DIRECTIVE_BSS){
        last_section = current_section;
        current_section = SECTION_BSS;
    }
    else if(dir_info->directive_id == DIRECTIVE_GLOBAL){
        asm_fp_directive_global(dir_info);
    }
    //All other directives require a section and only increment current section loc
    else{
        if(current_section == SECTION_UND){
            sprintf(error_msg_buffer, "Directive placed outside of any section");
            return 0;
        }
        /* Symbol directives only have semantic value when used with .word */
        if(dir_info->directive_id == DIRECTIVE_CHAR || dir_info->directive_id == DIRECTIVE_LONG){
            struct directive_arg* dir_arg = dir_info->args;
            while(dir_arg != NULL){
                if(dir_arg->type != DIRECTIVE_ARG_NUMBER){
                    sprintf(error_msg_buffer, "Symbolic arguments in directive only have meaning with .word");
                    return 0;
                }

                dir_arg = dir_arg->next;
            }
        }
        section_loc += asm_directive_get_size(dir_info);
    }
    
    //If we have changed the section, check if we have it in the symbol table
    if(dir_info->directive_id == DIRECTIVE_TEXT || dir_info->directive_id == DIRECTIVE_DATA || dir_info->directive_id == DIRECTIVE_RODATA || dir_info->directive_id == DIRECTIVE_BSS ){
        //Currently, one section of a given type per file is supported
        if(asm_symtab_get_symbol_entry(sections[current_section]) == -1){
            section_start_address += section_loc;
            asm_symtab_add_symbol(sections[current_section], current_section, section_start_address, LINKAGE_LOCAL, SYMTYPE_SECTION);
            if(last_section != SECTION_UND){
                uint16_t symbol = asm_symtab_get_symbol_entry(sections[last_section]);    
                symbol_table[symbol].size = section_loc;
            }
            section_loc = 0;
        }
        else{
            sprintf(error_msg_buffer, "Section %s has already been defined in this file", sections[current_section]);
            return 0;
        }
    }

    return 1;
}

int asm_fp_instruction(struct instruction_info* instr_info)
{
    uint8_t ret = asm_instruction_is_valid(instr_info);
    if(ret == INSTR_INVALID_OP_CNT){
        sprintf(error_msg_buffer, "Invalid instruction operand count");
        return 0;
    }
    if(ret == INSTR_INVALID_ADDRESS_MODES){
        sprintf(error_msg_buffer, "Instruction has invalid operand types");
        return 0;
    }
    if(ret == INSTR_IMMEDIATE_AS_DST){
        sprintf(error_msg_buffer, "Instruction has invalid destination");
        return 0;
    }
    if(current_section == SECTION_UND){
        sprintf(error_msg_buffer, "Instruction outside of a section");
        return 0;
    }

    section_loc += asm_instruction_get_size(instr_info);
    return 1;
}

void asm_fp_finish()
{
    if(current_section != SECTION_UND){
        uint16_t symbol = asm_symtab_get_symbol_entry(sections[current_section]);    
        symbol_table[symbol].size = section_loc;
    }
}

/*
static void print_op_info(struct operand_info* op_info)
{
    if(op_info->type == OPERAND_IMM){
        printf("Type: immediate Value: %d\n", op_info->imm.value);
    }
    else if(op_info->type == OPERAND_SYMBOL_VALUE){
        printf("Type: symbol value Symbol name: %s\n", op_info->symval.symname);
    }
    else if(op_info->type == OPERAND_SYMBOL_MEMDIR){
        printf("Type: symbol memdir Symbol name: %s\n", op_info->symmem.symname);
    }
    else if(op_info->type == OPERAND_ABSOLUTE_MEMDIR){
        printf("Type: absolute memdir Address: %d\n", op_info->absmem.addr);
    }
    else if(op_info->type == OPERAND_REGDIR){
        printf("Type: register direct Register: %d\n", op_info->regdir.reg);
    }
    else if(op_info->type == OPERAND_REGINDIR_ABSOLUTE){
        printf("Type: register indirect with disp Register: %d Disp: %d\n", op_info->regindir.reg, op_info->regindir.disp);
    }
    else if(op_info->type == OPERAND_REGINDIR_SYMBOL){
        printf("Type: register indirect with symbol disp Register %d Symbol: %s\n", op_info->regindirsym.reg, op_info->regindirsym.symname);
    }
    else if(op_info->type == OPERAND_RELATIVE_MEMDIR){
        printf("Type: symbol memdir relative Symbol name: %s\n", op_info->rel_memdir.symname);
    }
    else{
        printf("Unrecoverable operand print error.");
        exit(666);
    }
}
*/


static void asm_val_to_hex(uint32_t val, uint8_t bytes, char* output)
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

static void asm_print_output(uint8_t byte)
{
    static char buffer[] = {'\0', '\0', '\0'};
    asm_val_to_hex(byte, 1, buffer);
    
    fprintf(output_file_text, "%s ", buffer);
    ++output_byte_cnt;
    if(output_byte_cnt == BYTES_PER_LINE){
        output_byte_cnt = 0;
        fprintf(output_file_text, "\n");
    }

    fwrite(&byte, sizeof(uint8_t), 1, output_file_bin);
}

static void asm_print_section_change(uint16_t section)
{
    output_byte_cnt = 0;
    if(section != SECTION_BSS)
        fprintf(output_file_text, "\n# Section %s\n", sections[section]);
}


static void asm_sp_directive_align(struct directive_info* dir_info)
{
    uint16_t align = dir_info->args->num;
    while(section_loc % align != 0){
        ++section_loc;
        if(current_section != SECTION_BSS)
            asm_print_output(0);
    }
}

static void asm_sp_directive_skip(struct directive_info* dir_info)
{
    uint16_t bytes = dir_info->args->num;
    if(current_section != SECTION_BSS)
        for(uint16_t i = 0; i < bytes; ++i)
            asm_print_output(0);   

    section_loc += bytes;
}

static int asm_sp_directive_alloc(struct directive_info* dir_info)
{
    uint32_t target;
    uint8_t ticks;
    if(dir_info->directive_id == DIRECTIVE_CHAR)
        ticks = 1;
    else if(dir_info->directive_id == DIRECTIVE_WORD)
        ticks = 2;
    else
        ticks = 4; //Directive .long

    struct directive_arg* dir_arg = dir_info->args;
    while(dir_arg != NULL){
        if(dir_arg->type == DIRECTIVE_ARG_NUMBER){
            target = dir_arg->num;
        }
        else{
            int symbol_num = asm_symtab_get_symbol_entry(dir_arg->symname);
            if(symbol_num == -1){
                sprintf(error_msg_buffer, "Usage of undefined symbol %s (delcare with .global if external)", dir_arg->symname);
                return 0;
            }
            struct symtab* entry = symbol_table + symbol_num;
            int section_index = asm_symtab_get_section_entry(entry->section);

            if(entry->linkage == LINKAGE_LOCAL){
                target = entry->value;
                asm_reltab_add_relocation(current_section, section_loc, RELOCATION_ABSOLUTE, section_index);
            }
            else{
                target = 0;
                asm_reltab_add_relocation(current_section, section_loc, RELOCATION_ABSOLUTE, symbol_num);
            }
        }

        for(int i = 0; i < ticks; ++i){
            asm_print_output(target & 0xFF);
            target >>= 8;
        }

        section_loc += ticks;

        dir_arg = dir_arg->next;
    }

    return 1;
}

int asm_sp_directive(struct directive_info* dir_info)
{
    if(dir_info->directive_id == DIRECTIVE_TEXT){
        current_section = SECTION_TEXT;
    }
    else if(dir_info->directive_id == DIRECTIVE_DATA){
        current_section = SECTION_DATA;
    }
    else if(dir_info->directive_id == DIRECTIVE_RODATA){
        current_section = SECTION_RODATA;
    }
    else if(dir_info->directive_id == DIRECTIVE_BSS){
        current_section = SECTION_BSS;
    }
    else if(dir_info->directive_id == DIRECTIVE_GLOBAL){
        //No work in second pass
    }
    else if(dir_info->directive_id == DIRECTIVE_ALIGN){
        asm_sp_directive_align(dir_info);
    }
    else if(dir_info->directive_id == DIRECTIVE_SKIP){
        asm_sp_directive_skip(dir_info);
    }
    //Allocation directives
    else{
        return asm_sp_directive_alloc(dir_info);
    }
    //If we have changed the section, check if we have it in the symbol table
    if(dir_info->directive_id == DIRECTIVE_TEXT || dir_info->directive_id == DIRECTIVE_DATA || dir_info->directive_id == DIRECTIVE_RODATA || dir_info->directive_id == DIRECTIVE_BSS ){
        asm_print_section_change(current_section);
        section_loc = 0;
    }

    return 1;
}

static uint32_t asm_sp_instr_operand_code(struct operand_info* op_info)
{
    //Immediate
    if(op_info->type == OPERAND_IMM || op_info->type == OPERAND_SYMBOL_VALUE)
        return 0x00;
    //PSW
    if(op_info->type == OPERAND_REGDIR && op_info->regdir.reg == 8)
        return 0x07;
    //Memory direct
    if(op_info->type == OPERAND_SYMBOL_MEMDIR || op_info->type == OPERAND_ABSOLUTE_MEMDIR)
        return 0x10;
    //Register direct
    if(op_info->type == OPERAND_REGDIR)
        return 0x08 | op_info->regdir.reg;
    uint32_t reg;
    if(op_info->type == OPERAND_RELATIVE_MEMDIR )
        reg = REGISTER_PC;
    else if(op_info->type == OPERAND_REGINDIR_ABSOLUTE)
        reg = op_info->regindir.reg;
    else if(op_info->type == OPERAND_REGINDIR_SYMBOL)
        reg = op_info->regindirsym.reg;
    
    return 0x18 | reg;
}

int asm_sp_instruction(struct instruction_info* instr_info)
{
    uint16_t instr_word_first = 0;
    uint32_t cond_bits = instr_info->condition_id;
    uint32_t mnemonic_opcode = 0;
    
    if(instr_info->mnemonic_id < 16){
        mnemonic_opcode = instr_info->mnemonic_id;
    }
    else{
        //JMP
        if(instr_info->mnemonic_id == MNEMONIC_JMP){
            if(instr_info->first_op.type == OPERAND_RELATIVE_MEMDIR){
                mnemonic_opcode = instr_info->mnemonic_id = MNEMONIC_ADD;
                struct operand_info tmp;
                asm_parseh_copy_opinfo(&tmp, &instr_info->first_op);
                instr_info->first_op.type = OPERAND_REGDIR;
                instr_info->first_op.regdir.reg = REGISTER_PC;
                
                instr_info->second_op.type = OPERAND_IMM;
                int symbol_index = asm_symtab_get_symbol_entry(tmp.rel_memdir.symname);
                if(symbol_index == -1){
                    sprintf(error_msg_buffer, "Usage of undefined symbol %s (Declare with .global if extern)\n", tmp.rel_memdir.symname);
                    return 0;
                }
                struct symtab* entry = symbol_table + symbol_index;
                if(entry->linkage == LINKAGE_GLOBAL){
                    instr_info->second_op.imm.value = -2;
                    asm_reltab_add_relocation(current_section, section_loc + 2, RELOCATION_PC_RELATIVE, symbol_index);
                }
                else{
                    if(entry->section == current_section){
                        instr_info->second_op.imm.value = entry->value - (section_loc + 4);//Skip instruction + operand
                    }
                    else{
                        int section_index = asm_symtab_get_section_entry(entry->section);
                        instr_info->second_op.imm.value = entry->value - 2;
                        asm_reltab_add_relocation(current_section, section_loc + 2, RELOCATION_PC_RELATIVE, section_index);
                    }
                }
            }
            else{
                mnemonic_opcode = instr_info->mnemonic_id = MNEMONIC_MOV;
                asm_parseh_copy_opinfo(&instr_info->second_op, &instr_info->first_op);
                instr_info->first_op.type = OPERAND_REGDIR;
                instr_info->first_op.regdir.reg = REGISTER_PC;
            }
        }
        //RET
        else{
            mnemonic_opcode = instr_info->mnemonic_id = MNEMONIC_POP;
            instr_info->first_op.type = OPERAND_REGDIR;
            instr_info->first_op.regdir.reg = REGISTER_PC;
        }
    }

    uint32_t op_first_opcode = 0, op_second_opcode = 0;
    if(instr_op_cnt[instr_info->mnemonic_id] > 0)
        op_first_opcode = asm_sp_instr_operand_code(&instr_info->first_op);
    if(instr_op_cnt[instr_info->mnemonic_id > 1])
        op_second_opcode = asm_sp_instr_operand_code(&instr_info->second_op);
    

    instr_word_first = (cond_bits << 14) | (mnemonic_opcode << 10) | (op_first_opcode << 5) | op_second_opcode;
    asm_print_output(instr_word_first >> 8);
    asm_print_output(instr_word_first & 0xFF);
    section_loc += 2;

    struct operand_info* complex_op = NULL;
    if(instr_op_cnt[instr_info->mnemonic_id] > 0 && instr_info->first_op.type != OPERAND_REGDIR)
        complex_op = &instr_info->first_op;
    if(instr_op_cnt[instr_info->mnemonic_id] > 1 && instr_info->second_op.type != OPERAND_REGDIR)
        complex_op = &instr_info->second_op;
    
    if(complex_op != NULL){
        //Immediate
        if(complex_op->type == OPERAND_IMM){
            asm_print_output(complex_op->imm.value & 0xFF);
            asm_print_output( (complex_op->imm.value >> 8) & 0xFF);
        }
        //Register indirect by absolute value
        else if(complex_op->type == OPERAND_REGINDIR_ABSOLUTE){
            asm_print_output(complex_op->regindir.disp & 0xFF);
            asm_print_output( (complex_op->regindir.disp >> 8) & 0xFF);
        }
        //Memdir by absolute value
        else if(complex_op->type == OPERAND_ABSOLUTE_MEMDIR){
            asm_print_output(complex_op->absmem.addr & 0xFF);
            asm_print_output( (complex_op->absmem.addr >> 8) & 0xFF);
        }
        else {
            const char* symname;
            if(complex_op->type == OPERAND_REGINDIR_SYMBOL)
                symname = complex_op->regindirsym.symname;
            else if(complex_op->type == OPERAND_RELATIVE_MEMDIR)
                symname = complex_op->rel_memdir.symname;
            else if(complex_op->type == OPERAND_SYMBOL_MEMDIR)
                symname = complex_op->symmem.symname;
            else if(complex_op->type == OPERAND_SYMBOL_VALUE)
                symname = complex_op->symval.symname;
            
            int symbol_index = asm_symtab_get_symbol_entry(symname);
            if(symbol_index == -1){
                sprintf(error_msg_buffer, "Usage of undefined symbol %s (Declare with .global if extern)\n", symname);
                return 0;
            }

            if(complex_op->type == OPERAND_RELATIVE_MEMDIR){
                //Relative reltab
                if(symbol_table[symbol_index].linkage == LINKAGE_GLOBAL){
                    asm_reltab_add_relocation(current_section, section_loc, RELOCATION_PC_RELATIVE, symbol_index);
                    asm_print_output(0);
                    asm_print_output(0);
                }
                else{
                    int section_index = asm_symtab_get_section_entry(symbol_table[symbol_index].section);
                    int symval = symbol_table[symbol_index].value;

                    if(symbol_table[symbol_index].section == current_section){
                        symval -= section_loc + 2;
                    }
                    else{
                        symval -= 2;
                        asm_reltab_add_relocation(current_section, section_loc, RELOCATION_PC_RELATIVE, section_index);
                    }

                    asm_print_output(symval & 0xFF);
                    asm_print_output((symval >> 8) & 0xFF);;
                }
            }
            else{
                //Absolute reltab
                if(symbol_table[symbol_index].linkage == LINKAGE_GLOBAL){
                    asm_reltab_add_relocation(current_pass, section_loc, RELOCATION_ABSOLUTE, symbol_index);
                    asm_print_output(0);
                    asm_print_output(0);
                }
                else{
                    int section_index = asm_symtab_get_section_entry(symbol_table[symbol_index].section);
                    asm_reltab_add_relocation(current_pass, section_loc, RELOCATION_ABSOLUTE, section_index);
                    int symval = symbol_table[symbol_index].value;
                    asm_print_output(symval & 0xFF);
                    asm_print_output((symval >> 8) & 0xFF);;
                }
            }
        }
        section_loc += 2;
    }

    return 1;
}


/* 
                    Binary file layout
    SYMBOL TABLE
    SECTIONS
    RELOCATION TABLES

    Relocation tables and sections appear in the order they appear in the symbol table
*/

static void asm_binfile_print_header()
{
    fwrite(&symbol_cnt, sizeof(uint32_t), 1, output_file_bin);
    fwrite(symbol_table, sizeof(struct symtab), symbol_cnt, output_file_bin);
}

static int asm_symname_to_section(const char* symname)
{
    for(int i = 1; i < SECTION_CNT; ++i)
        if(strcmp(sections[i], symname) == 0)
            return i;

    return -1;
}

static void asm_binfile_print_reltables()
{
    for(int i = 0; i < symbol_cnt; ++i){
        int section = asm_symname_to_section(symbol_table[i].symname);
        if(section != -1 && section != SECTION_BSS){
            fwrite(&section_reltab[section - 1].cnt, sizeof(uint32_t), 1, output_file_bin);
            fwrite(section_reltab[section - 1].reldata, sizeof(struct reltab_entry), section_reltab[section - 1].cnt, output_file_bin);
        }
    }
}

static void asm_sp_finish()
{
    fprintf(output_file_text, "\n");
    for(int i = 0; i < RELOCATION_TABS_CNT; ++i)
        if(section_reltab[i].cnt != 0){
            fprintf(output_file_text, "# Relocation table for section %s\n", sections[i + 1]);
            struct reltab* reltab = section_reltab + i;
            const char* alloc_type[] = {"REL", "ABS"};
            char addr_buff[5];

            fprintf(output_file_text, "#ADDR TYPE SYM\n");
            for(int j = 0; j < reltab->cnt; ++j){
                asm_val_to_hex(reltab->reldata[j].offset, 2, addr_buff);
                fprintf(output_file_text, " %4s %4s %3d\n", addr_buff, alloc_type[reltab->reldata[j].type], reltab->reldata[j].symbol);
            }
        }

    const char* sections[] = {"UND", "TEXT", "DATA", "RODATA", "BSS"};
    const char linkage[] = { 'L', 'G' };
    
    fprintf(output_file_text, "# Symbol table\n");
    for(int i = 0; i < symbol_cnt; ++i){
        fprintf(output_file_text, " %2d. %10s %5d %7s %c %5d %s\n", i, symbol_table[i].symname, symbol_table[i].value, sections[symbol_table[i].section], linkage[symbol_table[i].linkage], symbol_table[i].size, symbol_table[i].type == SYMTYPE_SYM ? "SYM" : "SECTION" );
    }
    
    asm_binfile_print_reltables();
}

int yyparse();

int line_number;

extern int yylex_destroy(void);

int asm_do_file(const char* file, uint16_t section_start_addr)
{
    int i;
    current_file = fopen(file, "r");
    if(current_file == NULL){
        printf("The file %s does not exist or cannot be opened for reading.\n", file);
        return 0;
    }

    for(i = 0; i < SECTION_CNT - 2; ++i){
        section_reltab[i].reldata = NULL;
        section_reltab[i].cnt = section_reltab[i].size = 0;
    }
    
    current_file_path = file;
    yyin = current_file;
    line_number = 1;

    symbol_cnt = 0;
    symbol_table_size = 20;

    output_byte_cnt = 0;

    symbol_table = malloc(sizeof(struct symtab) * symbol_table_size);

    asm_symtab_add_symbol(sections[SECTION_UND], SECTION_UND, 0, LINKAGE_LOCAL, SYMTYPE_SECTION);

    current_section = SECTION_UND;
    section_loc = 0;
    section_start_address = section_start_addr;

    assembly_failed = 0;

    current_pass = FIRST_PASS;
    printf("Doing first pass\n");
    
    while(yyparse() == 0){
        ++line_number;
    }
    
    printf("First pass done.\n");

    if(!assembly_failed){
        char output_fname_buff[1000];
        strcpy(output_fname_buff, file);
        strcat(output_fname_buff, ".o.text");
        output_file_text = fopen(output_fname_buff, "w");
        if(output_file_text == NULL){
            fclose(current_file);
            printf("Cannot create output file %s\n", output_fname_buff);
            return 0;
        }

        strcpy(output_fname_buff, file);
        strcat(output_fname_buff, ".o");
        output_file_bin = fopen(output_fname_buff, "w");
        if(output_file_bin == NULL){
            fclose(current_file);
            fclose(output_file_text);
            printf("Cannot create output file(binary) %s\n", output_fname_buff);
            return 0;
        }
        
        asm_fp_finish();
        asm_binfile_print_header();
        current_pass = SECOND_PASS;
        line_number = 1;
        printf("Doing second pass\n");

        rewind(current_file);
        yylex_destroy();
        yyin = current_file;

        while(yyparse() == 0){
            ++line_number;
        }
        

        printf("Finished second pass\n");
        if(!assembly_failed){
            printf("Assembly success!\n");
            asm_sp_finish();
        }
        else
            printf("Assembly failed!\n");
        fclose(output_file_text);
        fclose(output_file_bin);
        yylex_destroy();
    }
    else{
        printf("First pass failed, skipping second pass.\n");
        yylex_destroy();
    }
    
    fclose(current_file);

    return 1;
}

void yyerror(const char* msg)
{
    printf("Parser error(%s) on line %d\n", msg, line_number);
    assembly_failed = 1;
}