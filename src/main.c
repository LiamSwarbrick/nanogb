#include "nanogb.h"
#include "opcode_mnem.h"

#include "raylib.h"  // for windowing, input and graphics

#include <stdio.h>
#include <string.h>

#define PLATFORM_DESKTOP  // probably don't need #define here but just to be sure
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION 330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION 100
#endif

int
main(int argc, char* argv[])
{
    // need to build newer version of raylib for up-to-date shader functions
    
    // init graphics (raylib)
    SetTraceLogLevel(0);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1920, 1080, "nanogb");
    SetTargetFPS(60);
    // load font: "res/Perfect DOS VGA 437.ttf"
    Font dos_font = LoadFont("res/Perfect DOS VGA 437.ttf");

    RenderTexture2D screen = LoadRenderTexture(160, 144);
    SetTextureFilter(screen.texture, FILTER_POINT);
    
    // init gb
    CPU cpu = cpu_create("testroms/gb_boot.gb");

    while (!WindowShouldClose())
    {
        BeginTextureMode(screen);  // emulate then render single frame to render texture
        ClearBackground(WHITE);
        
        while (!cpu_step(&cpu));

        DrawCircle(80, 72, cpu.pc, RED);

        EndTextureMode();


        // finally draw pixels to screen
        float scale = min((float)GetScreenWidth() / GB_WIDTH, (float)GetScreenHeight() / GB_HEIGHT);
        Rectangle dest_rect = { (GetScreenWidth() - ((float)GB_WIDTH * scale)) * 0.5, (GetScreenHeight() - ((float)GB_HEIGHT * scale)) * 0.5, (float)GB_WIDTH * scale, (float)GB_HEIGHT * scale };

        BeginDrawing();
        
        ClearBackground(BLACK);

        // draw render texture with letterboxing
        DrawTexturePro(screen.texture, (Rectangle){ 0.0f, 0.0f, screen.texture.width, screen.texture.width }, dest_rect, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // draw registers
        DrawTextEx(dos_font, TextFormat("AF:%X, BC:%X, DE:%X, HL:%X\nF:ZNHC0000\n  "PRINTF_BYTE_BIN_FMT"\nSP:%X, PC:%X", cpu.af, cpu.bc, cpu.de, cpu.hl, PRINTF_BYTE_BIN_VARS(cpu.f), cpu.sp, cpu.pc), (Vector2){ 0, 0 }, 24, 0, RAYWHITE);

        EndDrawing();
    }

    UnloadRenderTexture(screen);
    CloseWindow();
#if 0
    CPU cpu = cpu_create("testroms/gb_boot.gb");
    load_cart(&cpu, "testroms/dr_mario_world.gb");

    while (1)
    {
        u8 opcode = fetch_byte(&cpu);
        
        if (opcode != 0xCB)
        {
            printf("Opcode: 0x%X, %s\n", opcode, opcode_names_np[opcode]);
            execute_opcode(&cpu, opcode);
        }
        else  // cb opcode
        {
            opcode = fetch_byte(&cpu);
            printf("CB Opcode: 0x%X, %s\n", opcode, opcode_names_cb[opcode]);
            execute_cb_opcode(&cpu, opcode);
        }

    #if 0
        if (cycles % 1000 == 0)
            getchar();  // for stepping through instructions one by one
        printf("reg:\nA:%X\nF:ZNHC----\n  "PRINTF_BYTE_BIN_FMT"\nB:%X\nC:%X\nD:%X\nE:%X\nH:%X\nL:%X\n\n", cpu.a, PRINTF_BYTE_BIN_VARS(cpu.f), cpu.b, cpu.c, cpu.d, cpu.e, cpu.h, cpu.l);
        printf("AF:%X\nBC:%X\nDE:%X\nHL:%X\n\n", cpu.af, cpu.bc, cpu.de, cpu.hl);
        printf("SP:%X\nPC:%X\n__\n", cpu.sp, cpu.pc);
    #endif
    }
#endif
    return 0;
}
