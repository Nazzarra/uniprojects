#include <stdio.h>
#include <stdlib.h>
#include "asm_api.h"


int main(int argc, char* argv[])
{
    if(argc < 3){
        printf("Usage: filename addr [filename addr, filename addr, ...]");
        return 1;
    }

    int file_count = (argc - 1) / 2;
    int file_success = 0;

    for(int i = 0; i < file_count; ++i){
        const char* file_name = argv[1 + 2 * i];
        const int start_address = atoi(argv[2 + 2 * i]);
        printf("Assembling file: %s\nStarting address: %d\n", file_name, start_address);
        for(int i = 0; i < 30; ++i)
            printf("-");
        printf("\n");
        if(asm_do_file(file_name, start_address))
            ++file_success;
        printf("\n");
    }

    printf("Summary\n");
    for(int i = 0; i < 30; ++i)
        printf("-");
    printf("\nTotal files:%18d\nTotal assembled:%14d\n", file_count, file_success);

    return 0;
}