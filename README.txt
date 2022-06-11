# Audr32 (Audrey) CPU

Audr32 or Audrey, is a little 32-bit CPU I've been working on that I wish to make an operating system for sometime in the future. Audr32 supports a lot of essential features like:
    - Proper address mapping (See memory maps in vm/src/ISA.txt)
    - Pointers
    - Interrupts
    - I/O (Clock, Keyboard, Screen, Disk Drives)

Roadmap:

    - [X] Proper cycle operations
    - [X] Interrupts
    - [X] Basic memory mapping
    - [X] Screen
    - [X] Keyboard
    - [X] Proper address mapping
    - [X] Clock (For interval based interrupts and keeping time)
    - [X] Disk image loading
    - [-] Disk services interrupts
    - [ ] BIOS Firmware
    - [ ] Move interrupts to BIOS instead of embedding directly(?)
    - [ ] Basic game (inspired by my calculator programming)
    - [ ] Operating System and subsequentally a bootloader.


## Building

Building requires **SDL2** to be installed.

- Run `make` to build assembler and vm (emulator).

- Resulting executables will be in directories
    - ./vm/bin/vm
    - ./assembler/bin/assemble

## Documentation

Read `assembler/ASM.txt` and `vm/ISA.txt` to learn more about my insanity.
