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

// cpu.c probably needs reworking to be cleaner

int
cpu_step(CPU* cpu)
{
    u8 opcode = fetch_byte(cpu);
    
    if (opcode != 0xCB)
    {
        // printf("Opcode: 0x%X, %s\n", opcode, opcode_names_np[opcode]);
        execute_opcode(cpu, opcode);
    }
    else  // cb opcode
    {
        opcode = fetch_byte(cpu);
        // printf("CB Opcode: 0x%X, %s\n", opcode, opcode_names_cb[opcode]);
        execute_cb_opcode(cpu, opcode);
    }

    // update video
    cpu->video.mode_clock += cpu->delta_t_clock;
    switch (cpu->video.mode)
    {
        case SCANLINE_OAM:
            if (cpu->video.mode_clock >= SCANLINE_OAM_CLOCKS)
            {
                cpu->video.mode_clock = 0;  // reset clock
                cpu->video.mode = SCANLINE_VRAM;  // change mode
            }
            break;
        
        case SCANLINE_VRAM:
            if (cpu->video.mode_clock >= SCANLINE_VRAM_CLOCKS)
            {
                // end of scanline: write scanline to render texture (fbo)
                cpu->video.mode_clock = 0;
                cpu->video.mode = HBLANK;

                // render scaneline code...
                // I'll do that another day.
            }
            break;
        
        case HBLANK:
            if (cpu->video.mode_clock >= HBLANK_CLOCKS)
            {
                cpu->video.mode_clock = 0;
                
                if (++cpu->video.line == LAST_SCANLINE)
                {
                    cpu->video.mode = VBLANK;
                    return 1;  // return of 1 indicates full frame is rendered to texture and its time to swap buffers
                }
                else
                    cpu->video.mode = SCANLINE_OAM;
            }
            break;
        
        case VBLANK:
            if (cpu->video.mode_clock >= FULL_LINE_CLOCKS)
            {
                cpu->video.mode_clock = 0;
                ++cpu->video.line;

                if (cpu->video.line > LAST_VBLANK_LINE)
                {
                    cpu->video.mode = SCANLINE_OAM;
                    cpu->video.line = 0;  // back to top
                }
            }
            break;
    }
    return 0;
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
