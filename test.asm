; z80asm test.asm -o - | xxd -ps -c 16 > test.hex
org 0x0000

;ld sp,0x4000
;ld hl,0x2000
;ld (hl),0
;jp 0x0100

;ds 0x0100 - $,0xFF

;org 0x0100

ld a,0x7f
add a,1
ld a,0xFf
add a,1
ld a,-128
sub 1
sub 1
halt
