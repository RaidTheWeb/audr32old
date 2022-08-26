# Audr32 Assembler Specification

**Audr32 (Audrey) Assembly** (A32ASM) is a simple assembler targetting the Audr32 (Audrey) computer architecture.

A32ASM supports general assembly operations as well as Macros and other modern features.

## Output file structure:

Outputs can be given an offset with the `-b offset` flag on the assembler so that they may be placed at different locations in the address space, the offset is interpreted as base 16.
An output may also have a specified forced size with `-s size` (for flat binaries that can be loaded as disks in which case you need to allocate a number of sectors)

The user may specify a format with `-f format`, of these formats there is `bin` and `mf`, `bin` being a plain flat binary and `mf` being a more executable oriented file format containing entry symbol declarations and offset information among other things, it's designed for use in userspace executables.


Code section supports all generic instructions and directives.

Data section contains defined data such as:
- Strings (`.ascii`, `.asciiz` (`.string`))
- Binary Data (`.byte`, `.word`, `.dword`)

(Interrupt table would usually be created in the data section)
Data sections are not relocated to different areas of the executable and rather are inlined.

## Pointers:

A pointer can be used to reference the data located at a memory region.

Some examples of pointers follow:

    - Basic Pointer:
        `[0x330000]` (Will reference the 32 bit data value located at 0x330000 (RAM Start))
    - Basic Pointer With Size:
        `[16:0x40340000]` (Will reference the 16 bit data value located at 0x40340000 (Disk Drive Buffer Start))
    - Basic Pointer With Offset:
        `[32:0x333333:-4]` (Will reference the 32 bit data value located at the original address 0x333333 subtracted by 4 (0x33332F))
    - Register Pointers:
        `[bx]`
        `[8:ax]`
        `[32:bp:-24]` (Just so happens to be how local variables are implemented in CWCC)

## Macro Directives:

Macros are declared by a single `#` symbol at the start of a line. Valid macros include the following:

- `#define name value`                            - Declare a constant value (is not reflected in code whatsoever)
- `#org value`                                    - Modify offset (origin) for all following instructions
- `#include "path"`                               - Include another A32ASM file at current interpretation point.
- `#incbin "path"`                                - Include a binary file directly into output (only works in .data sections)
