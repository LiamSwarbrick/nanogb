#include "nanogb.h"

#include <stdio.h>
#include <math.h>

#include "opcode_mnem.h"

/* Data from https://github.com/drhelius/Gearboy/blob/master/src/opcode_timing.h */

const u8 opcode_machine_cycles[256] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,
    1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
    2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4,
    2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4,
    3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
    3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4
};

const u8 opcode_cb_machine_cycles[256] = {
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2
};


u8
fetch_byte(CPU* cpu)
{
    return cpu->mmu[cpu->pc++];
}

u16
fetch_word(CPU* cpu)
{
    u16 word = read_word(cpu->mmu, cpu->pc);
    cpu->pc += 2;  // increment pc by 2 bytes

    return word;
}

void
execute_opcode(CPU* cpu, u8 opcode)
{
    // 'JR NZ, n' add n to the program counter if the zero flag is not set

    // usefull rom disassembly with comments: https://gist.github.com/drhelius/6063288
    // opcode map: https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html
    switch (opcode)
    {
    case 0x0:  // NOP
        break;

    case 0x20:  // JR NZ,n
        if (!GET_BIT(cpu->f, F_Z))
        {
            s8 offset = fetch_byte(cpu);
            cpu->pc = (u16)((s16)cpu->pc + offset);
        }
        else
            ++cpu->pc;
        break;
    
    case 0x21:  // LD HL,d16
        cpu->hl = fetch_word(cpu);
        break;

    case 0x31:  // LD SP,d16
        cpu->sp = fetch_word(cpu);
        break;
    
    case 0x32:  // LD (HL-),A
        cpu->hl = cpu->a;
        --cpu->hl;
        break;
    
    case 0x7C:  // LD A,H
        cpu->a = cpu->h;
        break;
    
    case 0xAF:  // XOR A
        cpu->a = 0;  // xor with self = 0
        SET_BIT(cpu->f, F_Z);
        break;
    
    case 0xCB:  // PREFIX CB
        printf("This shouldn't appear, it would be weird if it does since cb opcodes are handled differently.");
        break;
    
    default:
        printf("Unimplemented opcode: 0x%X", opcode);
    }
}

void
execute_cb_opcode(CPU* cpu, u8 opcode)
{
#ifndef DEBUG_INSTRUCTIONS_OUTPUT
        printf("CB Opcode: 0x%X, %s\n", opcode, opcode_names_cb[opcode]);
#endif

    switch (opcode)
    {
        case 0x7C:  // BIT 7,H
            SET_BIT_TO(cpu->f, F_Z, GET_BIT(cpu->h, 7));
            RESET_BIT(cpu->f, F_N);
            SET_BIT(cpu->f, F_H);
            break;

        default:
            printf("Unimplemented CB opcode: 0x%X", opcode);
    }
}


