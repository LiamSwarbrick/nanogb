@echo off

REM maybe in future use dll and have application code load it at runtime

SET COMMON_FLAGS=-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-missing-braces -std=c99

PUSHD src

gcc %COMMON_FLAGS% *.c -o ../nano.exe

POPD

@echo on