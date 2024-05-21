; z80asm picalc.asm -o - | xxd -ps -c 16 > test.hex

ROMBASE:        equ 0
ROMSZ:          equ 8192
RAMBASE:        equ 8192
RAMSZ:          equ 8192

NBITS_INT:      equ 16
NBITS_FRAC:     equ 3368             ; O número de bits deve ser no mínimo 3.33x (1 / log(2)) o número de casas decimais + uma pequena folga
NBYTES_INT:     equ NBITS_INT >> 3
NBITS:          equ NBITS_INT+NBITS_FRAC
NBYTES:         equ NBITS>>3
NBYTES1:        equ NBYTES-1

    org ROMBASE

    ;ld sp,RAMBASE+RAMSZ
    ld sp,RAMBASE + RAMSZ
    call _main
    halt

    seek 0x0008     ; Para SW Interrupt
    org 0x0008
    ret

    seek 0x0038     ; Para HW Interrupt IM 1
    org 0x0038
    reti

    seek 0x0066     ; Para NMI
    org 0x0066
    retn

pidigits: db '3.14159265358979323846264338327950288419716939937510'
          db '58209749445923078164062862089986280348253421170679'
          db '82148086513282306647093844609550582231725359408128'
          db '48111745028410270193852110555964462294895493038196'
          db '44288109756659334461284756482337867831652712019091'
          db '45648566923460348610454326648213393607260249141273'
          db '72458700660631558817488152092096282925409171536436'
          db '78925903600113305305488204665213841469519415116094'
          db '33057270365759591953092186117381932611793105118548'
          db '07446237996274956735188575272489122793818301194912'
          db '98336733624406566430860213949463952247371907021798'
          db '60943702770539217176293176752384674818467669405132'
          db '00056812714526356082778577134275778960917363717872'
          db '14684409012249534301465495853710507922796892589235'
          db '42019956112129021960864034418159813629774771309960'
          db '51870721134999999837297804995105973173281609631859'
          db '50244594553469083026425223082533446850352619311881'
          db '71010003137838752886587533208381420617177669147303'
          db '59825349042875546873115956286388235378759375195778'
          db '18577805321712268066130019278766111959092164201989'

;///////////////////////////////////////////////////////////////////////////////
;   zero_reg
;   void zero_reg(uint8_t *reg);
;   Parâmetros:
;     HL: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF
zero_reg:

    ;    memset(reg,0,NBYTES);

    ld bc,NBYTES
    xor a
    ld (hl),a
    ld d,h
    ld e,l
    inc de
    ldir
    ret

;///////////////////////////////////////////////////////////////////////////////
;   set_bit_reg
;   void set_bit_reg(uint8_t *reg, int place);
;   Parâmetros:
;     HL: reg
;     BC: place
;   Retorna: Nada
;   Afeta: BC DE HL AF
set_bit_reg:

    ;   int byte = NBYTES1 - (place >> 3);
    push hl
    ld hl,NBYTES1

set_bit_reg0:

    ld d,b
    ld e,c
    srl d
    rr e
    srl d
    rr e
    srl d
    rr e
    scf
    ccf
    sbc hl,de
    ex de,hl    ;DE: byte
    pop hl

    ;   int bits = place & 0x07;
    ld a,c
    and 0x07    ;A: bits

    ;   reg[byte] |= 1<<bits;
    add hl,de
    ld b,1

set_bit_reg1:

    or a
    jr z, set_bit_reg2
    sla b
    dec a
    jr set_bit_reg1

set_bit_reg2:

    ld a,(hl)
    or b
    ld (hl),a
    ret

;///////////////////////////////////////////////////////////////////////////////
;   set_bit_reg_int
;   void set_bit_reg_int(uint8_t *reg, int place);
;   Parâmetros:
;     HL: reg
;     BC: place
;   Retorna: Nada
;   Afeta: BC DE HL AF
set_bit_reg_int:

    ;   int byte = NBYTES_INT - 1 - (place >> 3);
    push hl
    ld hl,NBYTES_INT
    dec hl
    jr set_bit_reg0

;///////////////////////////////////////////////////////////////////////////////
;   print_reg
;   void print_reg(uint8_t *reg);
;   Não portada

;///////////////////////////////////////////////////////////////////////////////
;   add_reg2_to_reg1
;   void add_reg2_to_reg1(uint8_t *reg1, const uint8_t *reg2);
;   Parâmetros:
;     HL: reg2
;     DE: reg1
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
add_reg2_to_reg1:

    ;   uint16_t cy = 0;
    ;   int i;
    ;   for (i = NBYTES1; i >= 0; i--){
    ;       uint16_t sum = (uint16_t)reg1[i] + (uint16_t)reg2[i] + cy;
    ;       if (sum & 0x100)
    ;           cy = 1;
    ;       else
    ;           cy = 0;
    ;       reg1[i] = sum & 0xff;
    ;   }

    ld bc,NBYTES1
    add hl,bc
    ex de,hl
    add hl,bc
    ex de,hl
    exx
    ld bc,NBYTES1
    inc bc
    exx
    sub a   ;zera CY

add_reg2_to_reg1_0:

    ld b,(hl)
    ld a,(de)
    adc a,b
    ld (de),a

    dec hl
    dec de

    exx                         ; Salva HL e DE, recupera contador em BC
    ex af,af'                   ; Salva CY
    dec bc
    ld a,b
    or c
    ret z                       ; Fim do loop, tchau
    ex af,af'                   ; Recupera CY
    exx                         ; Salva contador em BC, recupera HL e DE
    jr add_reg2_to_reg1_0

;///////////////////////////////////////////////////////////////////////////////
;   add_reg_to_acc
;   void add_reg_to_acc(const uint8_t *reg);
;   Parâmetros:
;     DE: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
add_reg_to_acc:

    ld hl,acc
    jp add_reg2_to_reg1

;///////////////////////////////////////////////////////////////////////////////
;   inc_reg_int8
;   void inc_reg_int8(uint8_t *reg, uint8_t byteval);
;   Parâmetros:
;     DE: reg
;     A:  byteval
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
inc_reg_int8:

    ;    uint16_t cy = byteval;
    ;    int i;
    ;    for (i = NBYTES_INT - 1; i >= 0; i--){
    ;        uint16_t sum =(uint16_t)reg[i] + cy;
    ;        reg[i] = sum & 0xff;
    ;        if (!(sum & 0x100))
    ;            return;
    ;        cy = 1;
    ;    }

    ld bc,NBYTES_INT
    ex de,hl
    add hl,bc
    dec hl
    ld e,a

inc_reg_int8_0:

    ld a,(hl)
    add a,e
    ld (hl),a

    ld e,0
    jr nc, inc_reg_int8_1
    inc e       ; Aqui usa E como CY.

inc_reg_int8_1:

    dec bc
    ld a,b
    or c
    ret z                       ; Fim do loop, tchau

    dec hl
    jr inc_reg_int8_0

;///////////////////////////////////////////////////////////////////////////////
;   load_reg_int
;   void load_reg_int(uint8_t *reg, int val);
;   Parâmetros:
;     DE: reg
;     HL: val
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
load_reg_int:

;    for (int i = NBYTES_INT - 1; i >= 0; i--){
;        reg[i] = val & 0xff;
;        val >>= 8;
;    }
    ex de,hl
    ld bc,NBYTES_INT
    add hl,bc
    dec hl
    ld b,NBYTES_INT
    ld a,e
    ld (hl),a
    dec hl
    ld a,d
    ld (hl),a
    dec b
    dec b
    ld a,b
    or a
    ret z
    dec hl
    xor a
load_reg_int_0:
    ld (hl),a
    dec hl
    djnz load_reg_int_0
    ret

;///////////////////////////////////////////////////////////////////////////////
;   sub_reg2_from_reg1
;   void sub_reg2_from_reg1(uint8_t *reg1, const uint8_t *reg2);
;   Parâmetros:
;     DE: reg1
;     HL: reg2
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'

sub_reg2_from_reg1:

;    uint16_t cy = 0;
;    int i;
;    for (i = NBYTES1; i >= 0; i--){
;        uint16_t diff = (uint16_t)reg1[i] - (uint16_t)reg2[i] - cy;
;        if (diff & 0x8000)
;            cy = 1;
;        else
;            cy = 0;
;        reg1[i] = diff & 0xff;
;    }

    ld bc,NBYTES1
    add hl,bc
    ex de,hl
    add hl,bc
    ex de,hl
    exx
    ld bc,NBYTES1
    inc bc
    exx
    sub a   ;zera CY

sub_reg2_to_reg1_0:

    ld b,(hl)
    ld a,(de)
    sbc a,b
    ld (de),a

    dec hl
    dec de

    exx                         ; Salva HL e DE, recupera contador em BC
    ex af,af'                   ; Salva CY
    dec bc
    ld a,b
    or c
    ret z                       ; Fim do loop, tchau
    ex af,af'                   ; Recupera CY
    exx                         ; Salva contador em BC, recupera HL e DE
    jr sub_reg2_to_reg1_0

;///////////////////////////////////////////////////////////////////////////////
;   sub_reg_from_acc
;   void sub_reg_from_acc(const uint8_t *reg);
;   Parâmetros:
;     HL: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'

sub_reg_from_acc:

    ld de,acc
    call sub_reg2_from_reg1
    ret

;///////////////////////////////////////////////////////////////////////////////
;   shl_reg
;   void shl_reg(uint8_t *reg, int places)
;   Parâmetros:
;     DE: reg
;     BC: places
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'

shl_reg:

;    int i;
;    if (places > NBITS)
;        places = NBITS;
;    int bytes = places >> 3;
;    int bits = places & 0x07;
;    int leftbytes = NBYTES - bytes;
;    if (bytes){
;        if (leftbytes)
;            memmove(&reg[0], &reg[bytes], leftbytes);
;        memset(&reg[leftbytes],0,bytes);
;    }
;    if (bits){
;        for (i = 0; i < (leftbytes-1); i++){
;            reg[i] <<= bits;
;            uint8_t aux = reg[i+1] >> (8-bits);
;            reg[i] |= aux;
;        }
;        reg[leftbytes - 1] <<= bits;
;    }

    ld ix,0xFFF8    ; Reserva 8 bytes
    add ix,sp
    ld sp,ix

    ld (ix+0),e     ;IX+0, IX+1 = 'reg'
    ld (ix+1),d

    ld a,NBITS >> 8
    sub b
    jr c,shl_reg_0
    jr nz,shl_reg_1

    ld a,NBITS & 255
    sub c
    jr nc, shl_reg_1

shl_reg_0:

    ld bc,NBITS

shl_reg_1:

    ld a,c
    and 0x07
    ld (ix+2),a     ; IX+2 = 'bits'
    srl b
    rr c
    srl b
    rr c
    srl b
    rr c            ; BC = 'bytes'
    ld hl,NBYTES
    xor a
    sbc hl,bc       ; HL = 'leftbytes'
    ld (ix+4),l     ; IX+4, IX+5 = 'leftbytes'
    ld (ix+5),h

    ld a,b
    or c
    jr z,shl_reg_2

    ld a,h
    or l
    jr z,shl_reg_1a

    ; Move bytes para a esquerda
    push bc         ; Salva 'bytes'
    push hl         ; Salva 'leftbytes'

    ld h,d
    ld l,e
    add hl,bc       ; DE='reg' HL='reg[bytes]'
    pop bc          ; Restaura 'leftbytes'
    ldir

    pop bc          ; Restaura 'bytes'

shl_reg_1a:

    ; Zera bytes da direita
    ; Aqui DE contém o primeiro byte a zerar (derivado do LDIR anterior, se ocorreu, ou o valor original).
    ld h,d
    ld l,e

shl_reg_1b:

    xor a
    ld (hl),a
    inc hl
    dec bc
    ld a,b
    or c
    jr nz,shl_reg_1b

shl_reg_2:

    ld a,(ix+2)     ; 'bits'
    or a
    jr z,shl_reg_end

    ; Vai fazer o shift
    ld c,(ix+4)
    ld b,(ix+5)     ;'leftbytes'

    ld l,(ix+0)
    ld h,(ix+1)     ; HL = 'reg'

shl_reg_2a:

    ld a,(ix+2)     ; 'bits'
    ld e,a          ; 'bits'
    neg
    add a,8
    ld d,a
    ld a,(hl)

shl_reg_2b:

    sla a
    dec e
    jr nz,shl_reg_2b

    ld e,a

    inc hl
    ld a,(hl)
    dec hl

shl_reg_2c:

    srl a
    dec d
    jr nz,shl_reg_2c

    or e
    ld (hl),a
    inc hl

    dec bc
    ld a,b
    or c
    jr nz,shl_reg_2a

    ; Ajusta o último byte relevante
    ld a,(hl)
    ld b,(IX+2)     ; 'bits'

shl_reg_2d:

    sla a
    djnz shl_reg_2d

    ld (hl),a

shl_reg_end:

    ld hl,0x0008    ; Libera 8 bytes
    add hl,sp
    ld sp,hl
    ret

;///////////////////////////////////////////////////////////////////////////////
stack_test:

    ld ix,0xFFF8    ; Reserva 8 bytes
    add ix,sp
    ld sp,ix

    ld c,(ix+0)
    ld b,(ix+1)
    ;....
    ld hl,0x0008    ; Libera 8 bytes
    add hl,sp
    ld sp,hl
    ret


;///////////////////////////////////////////////////////////////////////////////
;   shr_reg
;   void shr_reg(uint8_t *reg, int places);
;   Parâmetros:
;     DE: reg
;     BC: places
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
;
;    int i;
;
;    if (places > NBITS)
;        places = NBITS;
;
;    int bytes = places >> 3;
;    int bits = places & 0x07;
;
;    int rightbytes = NBYTES - bytes;
;
;    if (bytes){
;
;        if (rightbytes)
;            memmove(&reg[bytes], &reg[0], rightbytes);
;        memset(&reg[0],0,bytes);
;    }
;
;    if (bits){
;
;        for (i = NBYTES1; i > bytes; i--){
;
;            reg[i] >>= bits;
;            uint8_t aux = reg[i-1] << (8-bits);
;            reg[i] |= aux;
;        }
;
;        reg[bytes] >>= bits;
;    }

shr_reg:

    ld ix,0xFFF8    ; Reserva 8 bytes
    add ix,sp
    ld sp,ix

    ld (ix+0),e     ; IX+0, IX+1 = 'reg'
    ld (ix+1),d

    ld a,NBITS >> 8
    sub b
    jr c,shr_reg_0
    jr nz,shr_reg_1

    ld a,NBITS & 255
    sub c
    jr nc, shr_reg_1

shr_reg_0:

    ld bc,NBITS

shr_reg_1:

    ld a,c
    and 0x07
    ld (ix+2),a     ; IX+2 = 'bits'
    srl b
    rr c
    srl b
    rr c
    srl b
    rr c            ; BC = 'bytes'
    ld (ix+6),c
    ld (ix+7),b     ; IX+6, IX+7 = 'bytes'

    ld hl,NBYTES
    xor a
    sbc hl,bc       ; HL = 'rightbytes'
    ld (ix+4),l     ; IX+4, IX+5 = 'rightbytes'
    ld (ix+5),h

    ld a,b          ; if (bytes)
    or c
    jr z,shr_reg_2

    ld a,h          ; if (rightbytes)
    or l
    jr z,shr_reg_1a

    ; Move bytes para a direita

    ld hl,NBYTES1
    add hl,de
    ld d,h
    ld e,l          ; DE = último byte do reg (destino)

    xor a           ; BC = 'bytes'
    sbc hl,bc       ; HL = origem
    ld c,(ix+4)     ; 'rightbytes'
    ld b,(ix+5)
    lddr

shr_reg_1a:

    ; Zera bytes da esquerda

    ld c,(ix+6)
    ld b,(ix+7)     ; IX+6, IX+7 = 'bytes'
    ld l,(ix+0)
    ld h,(ix+1)     ; IX+0, IX+1 = 'reg'

shr_reg_1b:

    xor a
    ld (hl),a
    inc hl
    dec bc
    ld a,b
    or c
    jr nz,shr_reg_1b

shr_reg_2:

    ld a,(ix+2)     ; 'bits'
    or a
    jr z,shr_reg_end

    ; Vai fazer o shift

    ld hl,NBYTES1
    ld c,(ix+6)
    ld b,(ix+7)     ; IX+6, IX+7 = 'bytes'
    xor a
    sbc hl,bc
    ld b,h
    ld c,l          ; BC = número de bytes a processar

    ld l,(ix+0)
    ld h,(ix+1)     ; HL = 'reg'

    ld de,NBYTES1
    add hl,de

shr_reg_2a:

    ld a,(ix+2)     ; 'bits'
    ld e,a          ; E = 'bits'
    neg
    add a,8
    ld d,a          ; D = 8 - 'bits'
    ld a,(hl)

shr_reg_2b:

    srl a
    dec e
    jr nz,shr_reg_2b

    ld e,a

    dec hl
    ld a,(hl)
    inc hl

shr_reg_2c:

    sla a
    dec d
    jr nz,shr_reg_2c

    or e
    ld (hl),a
    dec hl

    dec bc
    ld a,b
    or c
    jr nz,shr_reg_2a

    ; Ajusta o último byte relevante
    ld a,(hl)
    ld b,(IX+2)     ; 'bits'

shr_reg_2d:

    srl a
    djnz shr_reg_2d

    ld (hl),a

shr_reg_end:

    ld hl,0x0008    ; Libera 8 bytes
    add hl,sp
    ld sp,hl
    ret

;///////////////////////////////////////////////////////////////////////////////
;   _main
;   void _main(void);
;   Parâmetros: Nada
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
_main:

;    call stack_test

;    ld hl,reg1
;    call zero_reg

;    ld bc,???
;    ld de,reg1
;    call shl_reg

    ld hl,reg1
    call zero_reg
    ld hl,reg1
    ld (hl),11h
    ld bc,NBYTES1
    add hl,bc
    ld (hl),11h

;loop:
;    ld de,reg1
;    ld bc,1
;    call shl_reg
;    jr loop

loop:
    ld de,reg1
    ld bc,1
    call shr_reg
    jr loop
    ;ret

;///////////////////////////////////////////////////////////////////////////////
    seek RAMBASE
    org RAMBASE

reg1:   ds NBYTES
acc:    ds NBYTES

    end

    ; Código "lixo" para teste do simulador
    ld bc,0x1111
    ld de,0x2222
    ld hl,0x3333
    ld ix,0x4444
    ld iy,0x5555
    push bc
    push de
    push hl
    push ix
    push iy

    pop bc
    pop de
    pop hl
    pop ix
    pop iy

    ld hl,0x20f4
    ld a,(hl)
    ld b,a
    ld c,a
    ld d,a
    ld e,a
    ld h,a
    ld l,a
    ld ixh,a
    ld ixl,a
    ld iyh,a
    ld iyl,a
    ld a,0xAA
    ld hl,0x20f4
    ld (hl),a
    dec hl
    ld (hl),0x88
    ld ix,0x2000
    ld iy,0x2040
    ld (ix+0),a
    ld (iy+2),a
