#include "nanogb.h"

#include <stdio.h>
#include <math.h>

#include "opcode_mnem.h"

/* These 2 arrays from https://github.com/drhelius/Gearboy/blob/master/src/opcode_timing.h */

const u8 opcode_instruction_length[256] = {
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

const u8 opcode_cb_instruction_length[256] = {
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

// these values were read from https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html
// "Duration of conditional calls and returns is different when action is taken or not."
// to solve this for opcodes with multiple cycle counts, I add the difference between the two cycles when the action path is lit (meaning: I add it in the opcode's implementation).

// ugh... manually typed these out
const u8 opcode_clock_cycles[256] = {
//   0   1   2   3   4   5   6   7    8   9   A   B   C   D   E   F
     4, 12,  8,  8,  4,  4,  8,  4,  20,  8,  8,  8,  4,  4,  8,  4,  // 0
     4, 12,  8,  8,  4,  4,  8,  4,  12,  8,  8,  8,  4,  4,  8,  4,  // 1
     8, 12,  8,  8,  4,  4,  8,  4,   8,  8,  8,  8,  4,  4,  8,  4,  // 2 
     8, 12,  8,  8, 12, 12, 12,  4,   8,  8,  8,  8,  4,  4,  8,  4,  // 3
     4,  4,  4,  8,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // 4
     4,  4,  4,  4,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // 5
     4,  4,  4,  4,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // 6
     8,  8,  8,  4,  8,  8,  4,  8,   4,  4,  4,  4,  4,  4,  8,  4,  // 7 
     4,  4,  8,  8,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // 8
     4,  4,  4,  4,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // 9
     4,  4,  4,  4,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // A
     4,  4,  4,  4,  4,  4,  8,  4,   4,  4,  4,  4,  4,  4,  8,  4,  // B
     8, 12, 12,  4, 12, 16,  8, 16,   8, 16, 12,  4, 12, 24,  8, 16,  // C 
     8, 12, 12, 16, 12, 16,  8, 16,   8, 16, 12,  0, 12,  0,  8, 16,  // D
    12, 12,  8,  0,  0, 16,  8, 16,  16,  4, 16,  0,  0,  0,  8, 16,  // E
    12, 12,  8,  4,  0, 16,  8, 16,  12,  8, 16,  4,  0,  0,  8, 16,  // F
};

const u8 opcode_cb_clock_cycles[256] = {
//  0  1  2  3  4  5   6  7  8  9  A  B  C  D   E  F
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 1
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 2 
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 3
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 4
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 5
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 6
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 7 
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 8
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 9
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // A
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // B
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // C 
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // D
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // E
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8   // F
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
    // (HL) means the value at the address of the value of the register
    // HL- means post decrement, you can guess the rest

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
        cpu->mmu[cpu->hl] = cpu->a;
        --cpu->hl;
        break;
    
    case 0x7C:  // LD A,H
        cpu->a = cpu->h;
        break;
    
    case 0xAF:  // XOR A
        cpu->a = 0;  // xor with self = 0
        SET_BIT(cpu->f, F_Z);
        break;
    
    case 0xCB:  // PREFIX CB is handled in another function
        break;
    
    default:
        printf("Unimplemented opcode: 0x%X", opcode);
    }

    cpu->delta_cycles = opcode_instruction_length[opcode];
    cpu->clock_cycles += cpu->delta_cycles;
}

void
execute_cb_opcode(CPU* cpu, u8 opcode)
{
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

    cpu->delta_cycles = opcode_cb_instruction_length[opcode];
    cpu->clock_cycles += cpu->delta_cycles;
}


