/*
Gameboy CPU: Sharp LR35902 core @ 4.19 MHz. Similar to the Z80.

Resolution 160x144 (20x18 tiles)

There are eight 8-bit registers (can be combined to be 16 bit):
    A, F,  // accumulator and flags
    B, C,
    D, E,
    H, L

PC and SP are actual 16 bit registers (as opposed to af, bc, de & hl);
    pc,  program counter
    sp   stack pointer

The z80 is little endian because if you were to store HL to memory, L would be written a byte before H.
If you were to read, L would be read from the address before H.
 */


#ifndef NANOGB_H
#define NANOGB_H


#include <stdint.h>

#define GB_WIDTH 160
#define GB_HEIGHT 144

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))

// printf formating helper
#define PRINTF_BYTE_BIN_FMT "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_BIN_VARS(byte)  \
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'), (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

// unsigned ints
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
// signed ints
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// bit manipulation macros
#define GET_BIT(BYTE, N) ((BYTE & (1 << N)) >> N)  // get nth bit
#define SET_BIT_TO(BYTE, N, VAL) (BYTE &= ~((VAL != 0) << N))  // sets Nth bit to VAL
#define SET_BIT(BYTE, N) (BYTE |= 0b1 << N)  // sets Nth bit to one
#define RESET_BIT(BYTE, N) (BYTE &= ~(0b1 << N))  // sets Nth bit to zero

typedef u8 MMU[0xFFFF];

// cpu (Sharp LR35902 core @ 4.19 MHz, similar to the Z80)
typedef struct CPU
{
    union { struct { u8 f; u8 a; }; u16 af; };  // A, F
    union { struct { u8 c; u8 b; }; u16 bc; };  // B, C
    union { struct { u8 e; u8 d; }; u16 de; };  // D, E
    union { struct { u8 l; u8 h; }; u16 hl; };  // H, L
    u16 pc;  // program counter
    u16 sp;  // stack pointer

    MMU mmu;  // MMU used to map virtual addresses with physical addresses

    u64 t_clock;  // every machine cycle takes exactly 4 T states
    u8 delta_t_clock;  // cycles for last instrution
    
    struct
    {
        u16 mode;
        u16 mode_clock;
        u16 line;  // current scanline
    } video;
}
CPU;

// flags are from the upper nibble of the F register (ZSHC0000)
enum flag_positions
{
    F_Z = 7,  // zero flag byte pos
    F_N = 6,  // subtract flag byte pos
    F_H = 5,  // half carry flag byte pos
    F_C = 4,  // carry flag byte pos
};

enum memory_map  // Memory map http://gameboy.mongenel.com/dmg/asmmemmap.html
{
    // memory locations
    BOOT_ROM               = 0x0000,
    CART_HEADER            = 0x0100,
    CART_ROM_BANK_0        = 0x0150,
    CART_ROM_BANK_X        = 0x4000,
    CHARACTER_RAM          = 0x8000,
    BG_MAP_DATA_1          = 0x9800,
    BG_MAP_DATA_2          = 0x9C00,
    CART_RAM               = 0xA000,
    INTERNAL_RAM_BANK_0    = 0xC000,
    INTERNAL_RAM_BANK_N    = 0xD000,
    ECHO_RAM               = 0xE000,
    OAM                    = 0xFE00,
    UNUSABLE_MEMORY        = 0xFEA0,
    HARDWARE_IO_REGISTERS  = 0xFF00,
    ZERO_PAGE              = 0xFF80,
    INTERRUPT_ENABLE_FLAG  = 0xFFFF,

    // sizes of sections
    BOOT_ROM_SIZE                = 0x00FF,
    CART_HEADER_SIZE             = 0x004F,
    CART_ROM_BANK_0_SIZE         = 0x3EAF,
    CART_ROM_BANK_X_SIZE         = 0x3FFF,
    CHARACTER_RAM_SIZE           = 0x17FF,
    BG_MAP_DATA_1_SIZE           = 0x03FF,
    BG_MAP_DATA_2_SIZE           = 0x03FF,
    CART_RAM_SIZE                = 0x1FFF,
    INTERNAL_RAM_BANK_0_SIZE     = 0x1FFF,
    INTERNAL_RAM_BANK_N_SIZE     = 0x0FFF,
    ECHO_RAM_SIZE                = 0x1DFF,
    OAM_SIZE                     = 0x009F,
    UNUSABLE_MEMORY_SIZE         = 0x00FF,
    HARDWARE_IO_REGISTERS_SIZE   = 0x007f,
    ZERO_PAGE_SIZE               = 0x007E
};


enum video_modes
{
    // modes
    SCANLINE_OAM = 2,
    SCANLINE_VRAM = 3,
    HBLANK = 0,
    VBLANK = 1,

    // clocks
    SCANLINE_OAM_CLOCKS = 80,
    SCANLINE_VRAM_CLOCKS = 172,
    HBLANK_CLOCKS = 204,
    VBLANK_CLOCKS = 4560,  // (10 lines)
    
    FULL_LINE_CLOCKS = 456,  // how long it takes for one line (scan and blank)
    FULL_FRAME_CLOCKS = 70224,  // total

    // after this scanline is rendered to texture, it can be rendered to the screen
    LAST_SCANLINE = 143,  // GB_HEIGHT - 1
    LAST_VBLANK_LINE = 153  // 10 vblank lines so LAST_SCANLINE + 10
};

// PROCS
void write_byte(MMU mmu, u16 addr, u8 v);
void write_word(MMU mmu, u16 addr, u16 v);
u8 read_byte(MMU mmu, u16 addr);
u16 read_word(MMU mmu, u16 addr);
int cpu_step(CPU* cpu);  // return of 1 indicates full frame is rendered to texture and its time to swap buffers
CPU cpu_create(const char* boot_rom_path);
void cpu_reset(CPU* cpu);
void load_cart(CPU* cpu, const char* filepath);

// opcodes
u8 fetch_byte(CPU* cpu);
u16 fetch_word(CPU* cpu);
void execute_opcode(CPU* cpu, u8 opcode);
void execute_cb_opcode(CPU* cpu, u8 opcode);

#endif  // NANOGB_H
