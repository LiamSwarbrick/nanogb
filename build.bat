@echo off

SET COMMON_FLAGS=-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-missing-braces -std=c99

PUSHD src

gcc %COMMON_FLAGS% *.c -o ../nano.exe -Iinclude -Llib -llibraylib

POPD

@echo on