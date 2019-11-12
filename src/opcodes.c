#include "nanogb.h"

#include <stdio.h>

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
    // opcode map: https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html
    switch (opcode)
    {
    case 0x0:  // NOP
        break;

    case 0x20:  // JR NZ,n
        ;  // <-- empty statement needed for var decl after label (C quirk)
        s8 offset = fetch_byte(cpu);
        cpu->pc = offset > 0 ? cpu->pc + offset : cpu->pc - offset;
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
        execute_cb_opcode(cpu, fetch_byte(cpu));
        break;

    default:
        printf("Unimplemented opcode: 0x%X", opcode);
    }
}

void
execute_cb_opcode(CPU* cpu, u8 opcode)
{
    switch (opcode)
    {
        default:
            printf("Unimplemented CB opcode: 0x%X", opcode);
    }
}


