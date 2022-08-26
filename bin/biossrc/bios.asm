; RiptideBIOS 0.1.0 (Audr32)

jmp main

; bios information, exists at 0x4033006 onwards
biostr:
    .asciiz "RiptideBIOS (Version 0.1.0-A Audr32)\n" ; welcome string/long form identifier
biosdate:
    .asciiz "05/07/22" ; last updated day
biosarch:
    .byte 32 ; architecture size (32 for audr32)
biosrev:
    .byte 'A' ; initial revision (A)
biosver:
    .asciiz "0.1.0" ; initial version

#org 0x40330000

main:
    sub sp, 0x100 ; allocate memory for static ram (since it's within stack we can safely do this)

    jmp bios_entry


#include "std.asm"

#define BOOTABLE_OFFSET 0x00330002

bios_entry:
    ; display welcome
    mov r0, biostr
    call puts
    
    jmp disk_search_init

puts:
    puts_print_loop:
        mov r10, 0x01
        cmp [8:r0], 0
        je puts_print_loop_end
        mov dx, [8:r0]
        push r0
        mov bx, 0x0F00
        int 0x10
        pop r0
        add r0, 1
        jmp puts_print_loop
    puts_print_loop_end:
        ret

puti:
    mov r10, 0x07
    mov dx, r0
    mov cx, 0
    mov bx, 0x0F00
    int 0x10
    ret

putc:
    mov r10, 0x01
    mov dx, r0
    mov bx, 0x0F00
    int 0x10
    ret 

disk_search_init:
    mov dx, 0x00 ; sector
disk_search:
    outx 0x5A, ax ; ensure we always check the first drive (maybe change this to check next after reaching end of sectors)
    outx 0x59, 0x04 ; poll drive (grab sector count)

    push dx
    inx 0x5A
    cmp dx, 0x01 ; disk exists
    jne disk_nodisk

    inx 0x5B ; grab sector count
    mov cx, dx ; ensure we can operate on it after we restore dx (sector count)
    
    pop dx ; restore dx for operation
    add dx, 1 ; so that it fits with postive integer style
    mov bx, cx
    cmp dx, cx ; compare
    mov cx, 0x00 ; clear cx for string interrupts later
    jg disk_nomoresectors ; disk does not have enough sectors!
    sub dx, 1 ; restore dx to use 0-start style

    outx 0x5A, 0x00 ; first drive
    outx 0x59, 0x00 ; select first drive

    outx 0x5A, dx ; select sector
    outx 0x59, 0x01 ; read block
    cmp [16:ADDR_SECTORCACHESTART], 0x5969 ; compare boot magic against start of sector
    je disk_load ; load and jump to newly loaded code
    add dx, 0x01 ; select next sector
    jmp disk_search ; jump back so we check the loop again


disk_load:
    ; mov r0, LOADING
    ; call puts

    mov r10, 0x01 ; load sector
    mov ax, 0x00
    mov bx, ADDR_RAMSTART ; load into ram
    ; don't need to move something into DX for sector as we're already setting that
    int 0x05 ; disk services
    jmp BOOTABLE_OFFSET

disk_nodisk: ; we have found no mounted disk! (there will be no others based on how audr32 works)
    mov r0, NODISK
    call puts
    jmp $

disk_nomoresectors: ; there is no more sectors to work with
    mov r0, NOMORESECTORS
    call puts
    add ax, 1
    jmp disk_search

bios_data:

    LOADING: .asciiz "Found Bootable Disk Media, Booting...\n"
    NODISK: .asciiz "Failed to boot from Hard Disk... No Disk Found!\n"
    NOMORESECTORS: .asciiz "No More Sectors, checking next Hard Disk!\n"
