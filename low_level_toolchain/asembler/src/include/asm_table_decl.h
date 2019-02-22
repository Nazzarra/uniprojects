#ifndef _ASM_TABLE_DECL_H
#define _ASM_TABLE_DECL_H

#include <stdint.h>
#include "asm_parse_decl.h"
#include "asm_constants.h"

struct symtab
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

struct reltab
{
    struct reltab_entry* reldata;
    uint32_t cnt;
    uint32_t size;
};

struct elf_file_header
{

};

struct elf_section_header
{

};

#endif