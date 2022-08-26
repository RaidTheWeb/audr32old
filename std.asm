;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Standard Headers and Functions           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Mapped address locations
#define ADDR_BUSREGISTERSSTART      0x000002F5
#define ADDR_BUSREGISTERSEND        0x000002FF
#define ADDR_FRAMEBUFFERSTART       0x00000300
#define ADDR_FRAMEBUFFEREND         0x0012C300
#define ADDR_TEXTMODESTART          0x0012C400
#define ADDR_TEXTMODEEND            0x0012CAD0
#define ADDR_STACKRAMSTART          0x00130000
#define ADDR_STACKRAMEND            0x0032FFFB
#define ADDR_RAMSTART               0x00330000
#define ADDR_RAMEND                 0x4032FFFB
#define ADDR_ROMSTART               0x40330000
#define ADDR_ROMEND                 0x4033FFFF
#define ADDR_SECTORCACHESTART       0x40340000
#define ADDR_SECTORCACHEEND         0x40340200

; General Exceptions
#define EXC_BUSERROR                0x01
#define EXC_BADADDR                 0x02
#define EXC_BADINST                 0x03

; Flag indices
#define FLAG_CF                     0x00
#define FLAG_ZF                     0x01
#define FLAG_SF                     0x02
#define FLAG_TF                     0x03
#define FLAG_IF                     0x04
#define FLAG_DF                     0x05
#define FLAG_OF                     0x06


