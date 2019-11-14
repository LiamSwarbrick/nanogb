#include "nanogb.h"
#include "opcode_mnem.h"

#include <stdio.h>
#include <stdlib.h>

u8
read_byte(MMU mmu, u16 addr)
{
    return mmu[addr];
}

u16
read_word(MMU mmu, u16 addr)
{
    return (u16)mmu[addr] | ((u16)mmu[addr + 1] << 8);
}

void
cpu_step(CPU* cpu)
{
    u8 opcode = fetch_byte(cpu);
        
    if (opcode != 0xCB)
    {
        printf("Opcode: 0x%X, %s\n", opcode, opcode_names_np[opcode]);
        execute_opcode(cpu, opcode);
    }
    else  // cb opcode
    {
        opcode = fetch_byte(cpu);
        printf("CB Opcode: 0x%X, %s\n", opcode, opcode_names_cb[opcode]);
        execute_cb_opcode(cpu, opcode);
    }


}

CPU
cpu_create(const char* boot_rom_path)
{
    CPU cpu = {0};
    
    FILE* fp;
    fp = fopen(boot_rom_path, "rb");
    if (fp == NULL)
    {
        printf("Failed to load boot rom ROM: %s\n", boot_rom_path);
        exit(1);
    }

    fread(cpu.mmu + BOOT_ROM, 1, BOOT_ROM_SIZE, fp);  // read boot rom into memory

    fclose(fp);

    return cpu;
}

void
cpu_reset(CPU* cpu)
{
    cpu->af = 0;
    cpu->bc = 0;
    cpu->de = 0;
    cpu->hl = 0;

    cpu->pc = 0;
    cpu->sp = 0;
}

void
load_cart(CPU* cpu, const char* filepath)
{
    FILE* fp;
    fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        printf("Failed to load ROM: %s\n", filepath);
        exit(1);
    }

    // read cart into memory
    fread(cpu->mmu + CART_ROM_BANK_0, 1, CART_ROM_BANK_0_SIZE, fp);
    fread(cpu->mmu + CART_ROM_BANK_X, 1, CART_ROM_BANK_X_SIZE, fp);

    fclose(fp);
}
