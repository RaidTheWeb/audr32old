# Audr32 ISA Specification

Audr32 (Audrey) is a 32-bit computer architecture that can be used for Language Backends and its main focus, Operating Systems.

## Instructions:

### Arithmetic

```
ADD                             - Addition
SUB                             - Subtraction
DIV                             - Division
IDIV                            - Signed Division
MUL                             - Multiplication

INC                             - Increment
DEC                             - Decrement
CMP                             - Compare operands
AND                             - Logical AND (AND)
                                - AND comparison without affecting operands (TEST)
SHL                             - Shift Left
SHR                             - Shift Right
XOR                             - Exclusive OR
OR                              - Logical OR
NOEG                            - Logical NOT (NOT)
                                - Negate (NEG)
```

### Stack

```
POP                             - Pop data from stack
PUSH                            - Push data onto stack
```

### I/O

```
INX                             - Input long (four bytes, double word) from port
OUTX                            - Output long (four bytes, double word) to port
```

### Procedures

```
CALL                            - Call a procedure (push IP register to stack)
    Essentially,
    PUSH IP
    MOV IP, procedurelocation
RET                             - Return from a procedure (set IP register to value on stack)
    Essentially,
    POP IP
```

### Miscellaneous

```
JMP                             - Unconditional Jump
    Essentially,
    MOV IP, location
JZ                              - Jump if zero
JNZ                             - Jump if not zero
JL                              - Jump if less than
JLE                             - Jump if less than or equal to
JG                              - Jump if greater than
JGE                             - Jump if greater than or equal to
SET                             - Set to 1 if equal, otherwise set to 0 (SETEQ)
                                - Set to 1 if not equal, otherwise set to 0 (SETNE)
                                - Set to 1 if less than, otherwise set to 0 (SETLT)
                                - Set to 1 if greater than, otherwise set to 0 (SETGT)
                                - Set to 1 if less than or equal to, otherwise set to 0 (SETLE)
                                - Set to 1 if greater than or equal to, otherwise set to 0 (SETGE)
LEA                             - Load the address of something into a register

MOV                             - Move data between pointers and registers among other things.

NOP                             - Perform no operation
HLT                             - Halt operation
INT                             - Call to interrupt
```

## Registers:

A register is a value stored in the CPU and not the memory, 
 a register can be used to store any 32 bit value for operation on.
Some instructions will not take registers as arguments and rather reference specific registers to draw and set data to.

The standard registers are as follows:

```
AX                              - 32 bit accumulator register
BX                              - 32 bit base register
CX                              - 32 bit counter register
DX                              - 32 bit data register
SI                              - 32 bit source index register
DI                              - 32 bit destination index register
SP                              - 32 bit stack pointer register
BP                              - 32 bit base pointer register
IP                              - 32 bit instruction pointer register
R0                              - Generic 32 bit register
R1                              - Generic 32 bit register
R2                              - Generic 32 bit register
R3                              - Generic 32 bit register
R4                              - Generic 32 bit register
R5                              - Generic 32 bit register
R6                              - Generic 32 bit register
R7                              - Generic 32 bit register
R8                              - Generic 32 bit register
R9                              - Generic 32 bit register
R10                             - Generic 32 bit register
R11                             - Generic 32 bit register
R12                             - Generic 32 bit register
R13                             - Generic 32 bit register
R14                             - Generic 32 bit register
R15                             - Generic 32 bit register
```

## I/O Ports: (16 bit integer)

```
0x0010                          - Serial 1
0x0011                          - Serial 1 CMD
0x0020                          - Keyboard
0x003A                          - Interrupt Port A (Await)
0x003B                          - Interrupt Port B (Load IDT)
0x0040                          - Clock Data
0x0041                          - Clock CMD
0x0059                          - Drive CMD
0x005A                          - Drive Data A
0x005B                          - Drive Data B
0x0060                          - Power Management Controller CMD
0x0070                          - Video Data Mode
0x0089                          - Audio CMD
0x008A                          - Audio Data A
0x008B                          - Audio Data B
```

## Interrupts: (8 bit integer)

Interrupts are CPU "events" triggered by an I/O device or code.
When an interrupt is called, any currently blocking HALT instructions are resolved and the CPU will go back to normal function.
A subroutine can be called in the event of an interrupt, these are declared in an interrupt table.
An interrupt table is located in memory and will only be configured if the code makes an OUTX call to the Interrupt Controller Device (IO Port 0x003B) with the following calls:

```x86asm
outx 0x003B, 0x000000FE ; (Opcode for setting an interrupt table)
outx 0x003B, memoryaddress ; (`memoryaddress` being the location of the first byte of the interrupt table.)
```

```
0x01                            - Keyboard Event
0x03                            - Clock Interval Event
0x04                            - Serial Event
0x05                            - Disk Drive Services
    - R10=0x01                  - Load Sector
        - AX                    - Drive Number
        - BX                    - Destination Address
        - DX                    - Sector Number
        Returns:
            - DX                - Destination Address
    - R10=0x02                  - Load Multiple sectors
        - AX                    - Drive Number
        - BX                    - Destination Start Address
        - DX                    - Start Sector
        - CX                    - Sector Count
        Returns:
            - DX                - Destination Start Address
0x10                            - Video Services
    - R10=0x01                  - Draw character (Teletype)
        - DX                    - Character
        - BX                    - Colour (See Colour Indexes)
    - R10=0x02                  - Draw String (Not recommended for proper display)
        - SI                    - Source address
        - CX                    - Length (If zero, read until we hit NULL)
        - BX                    - Colour (See Colour Indexes)
        - R8                    - Column (X)
        - R9                    - Row (Y)
    - R10=0x03                  - Clear Screen
        - BX                    - Colour (See Colour Indexes)
    - R10=0x04                  - Draw Pixel
        - BX                    - Colour (See Colour Indexes)
    - R10=0x05                  - Set Teletype Position
        - R8                    - Column (X)
        - R9                    - Row (Y)
    - R10=0x06                  - Convert Colour Index to AARRGGBB
        - BX                    - Colour (See Colour Indexes)
        Returns:
            - DX                - AARRGGBB Colour
    - R10=0x07                  - Print Integer (Helper)
        - DX                    - Number (32-bit Unsigned)
        - BX                    - Colour (See Colour Indexes)
0x14                            - Interrupt Services
    - R10=0x01                  - Load IDT
        - DX                    - Address of IDT
0x16                            - Keyboard Services
    - R10=0x01                  - Check Keystroke
        Returns:
            - R8                - Scancode
            - R9                - ASCII Character
            - ZF                - Available, set to 1 if there is no characters available
    - R10=0x02
        Returns:
            - R8                - Scancode
            - R9                - ASCII Character
    - R10=0x03
        - AX=0x01               - Only Accept Release
            - R8                - Enable
        - AX=0x02               - Only Accept Press
            - R8                - Enable
        - AX=0x03               - Clear Buffer
0x0E                            - Disk Operation Event
```

## Opcodes

```
    PREFIX
|<=-----------------------------------=>|
0                   8                   16                  24...
+-------------------+-------------------+-------------------+
| Instruction       | Mode              | Data...           |
+-------------------+-------------------+-------------------+
```

"Instruction" is the operation to commence (single 8 bit integer).
"Mode" is the specific mode for this operation, this can define what will be used.
    - There are modes for most specific operations.

## Pointers

A pointer references a location in memory. A pointer may reference an 8 bit integer, 16 bit integer, or 32 bit integer.

A standard pointer structure is as follows:

```
0               8               16               24               32               40               48               56               64                72
+---------------+-------------------------------------------------------------------+-------------------------------------------------------------------+
| Mode          | Address (32-bit)                                                  | Offset (32-bit Signed)                                            |
+---------------+-------------------------------------------------------------------+-------------------------------------------------------------------+
```

A standard register pointer structure is as follows:

```
0               8               16               24               32               40               48
+---------------+---------------+-------------------------------------------------------------------+
| Mode          | Register      | Offset (32-bit Signed)                                            |
+---------------+---------------+-------------------------------------------------------------------+
```

Pointer Modes:

0x01                            - 8 bit pointer
0x02                            - 16 bit pointer
0x03                            - 32 bit pointer
0x04                            - 8 bit register pointer
0x05                            - 16 bit register pointer
0x06                            - 32 bit register pointer

(Most data that is referenced to by a pointer is dereferenced and cast to a 32 bit datatype before working with it)

Pointers are not stored within any form of data, rather the data they reference is. Pointers are merely an abstraction that allow referencing a location in data and its value. Some instructions that will modify the contents of a pointer which will set the location in memory to the referenced value.


## Driver Specifications

### Keyboard

Audr32 uses an incredibly basic system for interpreting keycodes.

The scancodes are as follows:

```c
static uint8_t keymap[SDL_NUM_SCANCODES] = {
    [SDL_SCANCODE_A] = 0x01,
    [SDL_SCANCODE_B] = 0x02,
    [SDL_SCANCODE_C] = 0x03,
    [SDL_SCANCODE_D] = 0x04,
    [SDL_SCANCODE_E] = 0x05,
    [SDL_SCANCODE_F] = 0x06,
    [SDL_SCANCODE_G] = 0x07,
    [SDL_SCANCODE_H] = 0x08,
    [SDL_SCANCODE_I] = 0x09,
    [SDL_SCANCODE_J] = 0x0A,
    [SDL_SCANCODE_K] = 0x0B,
    [SDL_SCANCODE_L] = 0x0C,
    [SDL_SCANCODE_M] = 0x0D,
    [SDL_SCANCODE_N] = 0x0E,
    [SDL_SCANCODE_O] = 0x0F,
    [SDL_SCANCODE_P] = 0x10,
    [SDL_SCANCODE_Q] = 0x11,
    [SDL_SCANCODE_R] = 0x12,
    [SDL_SCANCODE_S] = 0x13,
    [SDL_SCANCODE_T] = 0x14,
    [SDL_SCANCODE_U] = 0x15,
    [SDL_SCANCODE_V] = 0x16,
    [SDL_SCANCODE_W] = 0x17,
    [SDL_SCANCODE_X] = 0x18,
    [SDL_SCANCODE_Y] = 0x19,
    [SDL_SCANCODE_Z] = 0x1A,

    [SDL_SCANCODE_1] = 0x1B,
    [SDL_SCANCODE_2] = 0x1C,
    [SDL_SCANCODE_3] = 0x1D,
    [SDL_SCANCODE_4] = 0x1E,
    [SDL_SCANCODE_5] = 0x1F,
    [SDL_SCANCODE_6] = 0x20,
    [SDL_SCANCODE_7] = 0x21,
    [SDL_SCANCODE_8] = 0x22,
    [SDL_SCANCODE_9] = 0x23,
    [SDL_SCANCODE_0] = 0x24,

    [SDL_SCANCODE_RETURN] = 0x50,
    [SDL_SCANCODE_ESCAPE] = 0x26,
    [SDL_SCANCODE_BACKSPACE] = 0x27,
    [SDL_SCANCODE_TAB] = 0x28,
    [SDL_SCANCODE_SPACE] = 0x29,

    [SDL_SCANCODE_MINUS] = 0x2A,
    [SDL_SCANCODE_EQUALS] = 0x2B,
    [SDL_SCANCODE_LEFTBRACKET] = 0x2C,
    [SDL_SCANCODE_RIGHTBRACKET] = 0x2D,
    [SDL_SCANCODE_BACKSLASH] = 0x2E,
    [SDL_SCANCODE_NONUSBACKSLASH] = 0x2F,

    [SDL_SCANCODE_SEMICOLON] = 0x30,
    [SDL_SCANCODE_APOSTROPHE] = 0x31,
    [SDL_SCANCODE_GRAVE] = 0x32,
    [SDL_SCANCODE_COMMA] = 0x33,
    [SDL_SCANCODE_PERIOD] = 0x34,
    [SDL_SCANCODE_SLASH] = 0x35,

    [SDL_SCANCODE_F1] = 0x36,
    [SDL_SCANCODE_F2] = 0x37,
    [SDL_SCANCODE_F3] = 0x38,
    [SDL_SCANCODE_F4] = 0x39,
    [SDL_SCANCODE_F5] = 0x3A,
    [SDL_SCANCODE_F6] = 0x3B,
    [SDL_SCANCODE_F7] = 0x3C,
    [SDL_SCANCODE_F8] = 0x3D,
    [SDL_SCANCODE_F9] = 0x3E,
    [SDL_SCANCODE_F10] = 0x3F,
    [SDL_SCANCODE_F11] = 0x40,
    [SDL_SCANCODE_F12] = 0x41,

    [SDL_SCANCODE_INSERT] = 0x42,
    [SDL_SCANCODE_HOME] = 0x43,
    [SDL_SCANCODE_PAGEUP] = 0x44,
    [SDL_SCANCODE_DELETE] = 0x45,
    [SDL_SCANCODE_END] = 0x46,
    [SDL_SCANCODE_PAGEDOWN] = 0x47,
    [SDL_SCANCODE_RIGHT] = 0x48,
    [SDL_SCANCODE_LEFT] = 0x49,
    [SDL_SCANCODE_DOWN] = 0x4A,
    [SDL_SCANCODE_UP] = 0x4B,

    [SDL_SCANCODE_KP_DIVIDE]   = 0x4C,
    [SDL_SCANCODE_KP_MULTIPLY] = 0x4D,
    [SDL_SCANCODE_KP_MINUS]    = 0x4E,
    [SDL_SCANCODE_KP_PLUS]     = 0x4F,
    [SDL_SCANCODE_KP_ENTER]    = 0x50,
    [SDL_SCANCODE_KP_1]        = 0x51,
    [SDL_SCANCODE_KP_2]        = 0x52,
    [SDL_SCANCODE_KP_3]        = 0x53,
    [SDL_SCANCODE_KP_4]        = 0x54,
    [SDL_SCANCODE_KP_5]        = 0x55,
    [SDL_SCANCODE_KP_6]        = 0x56,
    [SDL_SCANCODE_KP_7]        = 0x57,
    [SDL_SCANCODE_KP_8]        = 0x58,
    [SDL_SCANCODE_KP_9]        = 0x59,
    [SDL_SCANCODE_KP_0]        = 0x5A,
    [SDL_SCANCODE_KP_PERIOD]   = 0x5B,

    [SDL_SCANCODE_LCTRL]  = 0x60,
    [SDL_SCANCODE_LSHIFT] = 0x61,
    [SDL_SCANCODE_LALT]   = 0x62,
    [SDL_SCANCODE_LGUI]   = 0x63,
    [SDL_SCANCODE_RCTRL]  = 0x64,
    [SDL_SCANCODE_RSHIFT] = 0x65,
    [SDL_SCANCODE_RALT]   = 0x66,
    [SDL_SCANCODE_RGUI]   = 0x67,
};
```

If the key is pressed down it'll be OR'd (`|`) with 0x80 meaning to ensure you always get the scancode you must use `scancode & 0x7F`

If you wish to grab a scancode directly from the I/O device (in case you're following up on an interrupt trigger) you'd simply need to `inx 0x20` and the keyboard scancode will be put into the `DX` register while also removing the scancode from the internal buffer.

Alternatively, you can use interrupts to both check for said scancodes and grab them, the first of which can help you make sure a scancode exists.

### Screen

The display system for Audr32 is incredibly simple, using well established implementations for both framebuffer and text mode.

#### Setting Modes

- Text Mode (80x30): 0xEFEFEFEF
- Framebuffer Mode (AARRGGBB Full Colour Framebuffer (640x480)): 0x16161616

To set the screen mode, simply `outx 0x70, screenmode` which will change the bus register for the screen mode to the new mode.

#### Framebuffer Mode

Every pixel (memory location 0x00000300 and onwards, ending at 0x0012C300) is in the generic AARRGGBB format, meaning that white would be 0x00FFFFFF and blue would be 0x000000FF. The full framebuffer possesses a full 640x480 resolution.

#### Text Mode

Text Mode in audr32 is largely modeled off of x86's CGA implementation, with a few key changes, namely the length being 3 bytes in comparison to 2 for every character. Text Mode possesses 80 columns and 30 rows each containing 3 bytes to tell the screen driver what to interpret as.

Structure is as follows:
```
0               8               16               24
+---------------+---------------+----------------+
| Character     | Foreground    | Background     |
+---------------+---------------+-----------------
```

Foreground and background colours are both a single byte, giving the opportunity for full 16-bit colour.

```c
static uint32_t colours[0xF + 1] = {
    0x00000000, // black
    0x000000DD, // blue
    0x0000DD00, // green
    0x0000DDDD, // cyan
    0x00DD0000, // red
    0x00DD00DD, // magenta
    0x00DDDD00, // brown
    0x009B9B9B, // light grey
    0x004A4A4A, // dark grey
    0x000000FF, // blue
    0x0000FF00, // green
    0x0000FFFF, // cyan
    0x00FF0000, // red
    0x00FF00FF, // magenta
    0x00FFFF00, // brown
    0x00FFFFFF, // white
};
```

### Disk

Audr32 can read and write from as many as 8 disk drives at any time. (multiple can be passed into the emulator with the `-disk image` argument) the `image` has to be a valid, existing disk image with at least 512 bytes in size (otherwise it will not interpret any blocks from it, stupid I know).

Sectors are 512 bytes each, at least one must be present in a disk drive for it to work properly. Sectors are zero-started in all disk interrupts and I/O services.

#### Disk I/O Services

Set ModA with `outx 0x5A, value` and ModB with `outx 0x5B, value`.


Commands (I/O Port: 0x59):
- Select Drive: 0x00
    - ModA: Drive Number
- Read Sector/Block: 0x01
    - ModA: Sector/Block Number
- Write Sector/Block: 0x02
    - ModA: Sector/Block/Number
- Info: 0x03
    - Sets:
        - ModA: Info
        - ModB: Details
- Poll: 0x04
    - ModA: Drive Number
    - Sets:
        - ModA: Drive Exists
        - ModB: Sector/Block Count
- Enable Interrupts: 0x05
- Disable Interrupts: 0x06

#### Disk Interrupts

- Load a single sector to a location: 0x01
    - AX: Drive Number
    - BX: Destination Address
    - DX: Sector/Block Number
    - Sets:
        - DX: Destination Address
- Load a number of sectors to a location: 0x02
    - AX: Drive Number
    - BX: Destination Address
    - CX: Sector/Block Count
    - DX: Sector/Block Start
    - Sets:
        - DX: Destination

### Clock

Audr32 has a built-in clock I/O device that can be queried for time *and* trigger interrupts at a programmable interval:.

#### Clock I/O Services

Set Mod with `outx 0x40, value`.

Commands (I/O Port: 0x41): 
- Set Programmable Interval: 0x00
    - Mod: Interval (Milliseconds)
- Get Epoch Seconds: 0x01
    - Sets:
        - Mod: Epoch Seconds
- Get Epoch Milliseconds: 0x02
    - Sets:
        - Mod: Epoch Milliseconds (Incredibly rough estimation from seconds)
- Set Epoch Seconds: 0x03
    - Mod: Epoch Seconds
- Set Epoch Milliseconds: 0x04
    - Mod: Epoch Milliseconds
- Get Year: 0x05
    - Mod: Year
- Get Month: 0x06
    - Mod: Month
- Get Day: 0x07
    - Mod: Day
- Get Hour: 0x08
    - Mod: Hour
- Get Minute: 0x09
    - Mod: Minute
- Get Second: 0x0A
    - Mod: Second

### Power Manager

There exists an incredibly simple power manager with only one I/O command to shutdown the CPU.

#### Power I/O Services

Commands (I/O Port: 0x60):
- Turn CPU Off: 0x00

### Serial

It is possible to manage I/O through a serial port, rather than a screen and a keyboard. For these use cases audr32 has a serial system that can manage this I/O easily.

#### Serial I/O Services

Commands Serial 1 (I/O Port: 0x11):
- Enable Interrupts: 0x00
- Disable Interrupts: 0x01
- (Read) Get Busy State

Data Serial 1 (I/O Port: 0x10):
- (Write) Write Serial
- (Read) Read Serial

### Interrupt Controller

Audr32 has a seperate controller to handle interrupts for the CPU, it both provides interrupts and I/O services in order to manipulate this "chip"/controller.

#### Interrupt Controller I/O Services

- (Write) Wait For Interrupt (I/O Port: 0x3A)
    - Data: Interrupt Number
- (Read) Read Interrupt Busy (I/O Port: 0x3A) (Usually implausible)
    - Sets:
        - Return: Interrupt Busy State
- (Write) Set IDT (I/O Port: 0x3B)
    - Data: IDT Address

#### Interrupt Controller Interrupts

- Load IDT: 0x01
    - DX: IDT Address

### Audio Controller

Audr32 has a virtually "onboard" chip to control the speaker using I/O. It possesses 1 channel (mono) with a sample rate of 44.1 KHz (44100 Hz). Samples are in a 8-bit unsigned format with a sample count of 2048.

Playing a sound is as simple as filling in ModA with a pointer to some data (The audio that you wish to play) and ModB with the length of the aforementioned data, then calling the `Load Data` I/O Services command (The BIOS beep is actully implemented using an selection of varying sized square waves and this I/O command pointing to the corresponding wave), finally setting ModA to 0 and calling the `Modify Pause State` command for the sound to come out of the speaker.

#### Audio Controller I/O Services

Set ModA with `outx 0x8A, value` and ModB with `outx 0x8B, value`.

Commands (I/O Port: 0x89):
- Load Data: 0x00
    - ModA: Data Address
    - ModB: Data Length
- Destroy: 0x01
- Modify Pause State: 0x02
    - ModA: Pause State
- Load Beep: 0x03
    - ModA: Tone Frequency (Hz)
    - ModB: Duration (ms)
