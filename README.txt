# Audr32 (Audrey) CPU

A 32 bit CPU I've been working on for a bit, trying to make it run its own Operating System written in C or something.
Audr32 is inspired by other ISAs like x86, limn2600, and jdh8.

## Building

Building requires **SDL2** to be installed.

- Run `make` to build assembler and vm (emulator).

- Resulting executables will be in directories
    - ./vm/bin/vm
    - ./assembler/bin/assemble

## Documentation

Read `assembler/ASM.txt` and `vm/ISA.txt` to learn more about my insanity.