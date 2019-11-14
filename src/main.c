#include "nanogb.h"
#include "opcode_mnem.h"

#include <stdio.h>
#include <string.h>

int
main(int argc, char* argv[])
{
    CPU cpu = cpu_create("testroms/gb_boot.gb");

    while (1)
    {
        u8 opcode = fetch_byte(&cpu);
        
        if (opcode != 0xCB)
        {
#ifdef DEBUG_INSTRUCTIONS_OUTPUT
            printf("Opcode: 0x%X, %s\n", opcode, opcode_names_np[opcode]);
#endif
        execute_opcode(&cpu, opcode);
        }
        else  // cb opcode
        {
            opcode = fetch_byte(&cpu);
#ifdef DEBUG_INSTRUCTIONS_OUTPUT
            printf("CB Opcode: 0x%X, %s\n", opcode, opcode_names_cb[opcode]);
#endif 
            execute_cb_opcode(&cpu, opcode);
        }

        getchar();  // for stepping through instructions one by one

        printf("reg:\nA:%X\nF:ZNHC----\n  "PRINTF_BYTE_BIN_FMT"\nB:%X\nC:%X\nD:%X\nE:%X\nH:%X\nL:%X\n\n", cpu.a, PRINTF_BYTE_BIN_VARS(cpu.f), cpu.b, cpu.c, cpu.d, cpu.e, cpu.h, cpu.l);
        printf("AF:%X\nBC:%X\nDE:%X\nHL:%X\n\n", cpu.af, cpu.bc, cpu.de, cpu.hl);
        printf("SP:%X\nPC:%X\n__\n", cpu.sp, cpu.pc);
    }

    return 0;
}
