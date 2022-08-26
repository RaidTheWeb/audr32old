# Audr32 (Audrey) CPU

<img src="logo.png">
<hr>

Audr32 or Audrey, is a little 32-bit CPU I've been working on that I wish to make an operating system for sometime in the future. Audr32 supports a few essential features like:
- Proper address mapping (See memory maps in vm/src/ISA.txt)
- Pointers
- Interrupts
- Exceptions
- I/O (Clock, Keyboard, Screen, Disk Drives, and Audio.)

## Road Map

- [X] Proper cycle operations
- [X] Interrupts
- [X] Basic memory mapping
- [X] Screen
- [X] Keyboard
- [X] Proper address mapping
- [X] Clock (For interval based interrupts and keeping time)
- [X] Disk image loading
- [X] Disk services interrupts
- [X] Audio
- [ ] BIOS Firmware
- [ ] Move interrupts to BIOS instead of embedding directly(?)
- [ ] Basic game (inspired by my calculator programming)
- [ ] Operating System and before that: A bootloader


## Building

Building requires **SDL2** to be installed.

- Run `make` to build assembler and vm (emulator).

- Resulting executables will be in directories
    - `./vm/bin/vm`
    - `./assembler/bin/assemble`

## Documentation

Read [ISA Documentation/Spec (ISA.md)](vm/ISA.md) and [Assembler Documentation/Spec (ASM.md)](assembler/ASM.md) to learn more about my insanity.
