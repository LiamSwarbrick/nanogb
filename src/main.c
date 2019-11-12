#define NANOGB_DEBUG
#include "opcode_mnem.h"

#include "nanogb.h"

#include <stdio.h>
#include <string.h>

int
main(int argc, char* argv[])
{
    CPU cpu = cpu_create("testroms/gb_boot.gb");

    while (1)
    {
        u8 opcode = fetch_byte(&cpu);
        if (opcode == 0xCB)
            opcode = fetch_byte(&cpu);
        
#ifdef NANOGB_DEBUG
        if (opcode != 0xCB)
            printf("Opcode: 0x%X, %s\n", opcode, opcode_names_np[opcode]);
        else
            printf("Opcode: 0x%X, %s\n", opcode, opcode_names_cb[opcode]);
#endif
        execute_opcode(&cpu, opcode);
        
        getchar();  // for stepping through instructions one by one

        printf("reg:\nA:%X\nF:ZNHC----\n  "PRINTF_BYTE_BIN_FMT"\nB:%X\nC:%X\nD:%X\nE:%X\nH:%X\nL:%X\n\n", cpu.a, PRINTF_BYTE_BIN_VARS(cpu.f), cpu.b, cpu.c, cpu.d, cpu.e, cpu.h, cpu.l);
        printf("SP:%X\nPC:%X\n__\n", cpu.sp, cpu.pc);
    }

    return 0;
}
