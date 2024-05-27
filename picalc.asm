; z80asm picalc.asm -o - | xxd -ps -c 16 > test.hex

ROMBASE:        equ 0
ROMSZ:          equ 8192
RAMBASE:        equ 8192
RAMSZ:          equ 8192

NUM_IT:         equ 100
NUM_DECS:       equ 120
NBITS_FR:       equ (3*NUM_DECS + NUM_DECS >> 1)

NBITS_INT:      equ 16
NBITS_FRAC:     equ 8*(1 + NBITS_FR >> 3) ;3368             ; O número de bits deve ser no mínimo 3.33x (1 / log(2)) o número de casas decimais + uma pequena folga
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

pidigits: db "3.14159265358979323846264338327950288419716939937510"
          db "58209749445923078164062862089986280348253421170679"
          db "82148086513282306647093844609550582231725359408128"
          db "48111745028410270193852110555964462294895493038196"
          db "44288109756659334461284756482337867831652712019091"
          db "45648566923460348610454326648213393607260249141273"
          db "72458700660631558817488152092096282925409171536436"
          db "78925903600113305305488204665213841469519415116094"
          db "33057270365759591953092186117381932611793105118548"
          db "07446237996274956735188575272489122793818301194912"
          db "98336733624406566430860213949463952247371907021798"
          db "60943702770539217176293176752384674818467669405132"
          db "00056812714526356082778577134275778960917363717872"
          db "14684409012249534301465495853710507922796892589235"
          db "42019956112129021960864034418159813629774771309960"
          db "51870721134999999837297804995105973173281609631859"
          db "50244594553469083026425223082533446850352619311881"
          db "71010003137838752886587533208381420617177669147303"
          db "59825349042875546873115956286388235378759375195778"
          db "18577805321712268066130019278766111959092164201989"

;///////////////////////////////////////////////////////////////////////////////
;   prints
;   void prints(const char *string);
;   Parâmetros: HL: string
;   Retorna: Nada
;   Afeta: HL
prints:

    pop hl
prints_1:
    ld a,(hl)
    or a
    jr z,prints_2
    out(0),a
    inc hl
    jr prints_1
prints_2:
    inc hl
    push hl
    ret

;///////////////////////////////////////////////////////////////////////////////
;   print_crlf
;   void print_crlf(void);
;   Parâmetros: Nada
;   Retorna: Nada
;   Afeta: HL
print_crlf:

    call prints
    db 13,10,0
    ret

;///////////////////////////////////////////////////////////////////////////////
;   zero_reg
;   void zero_reg(uint8_t *reg);
;   Parâmetros:
;     HL: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF
zero_reg:

    ;    memset(reg,0,NBYTES);

    ld bc,NBYTES-1
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
    or a
    jr z,set_bit_reg2

set_bit_reg1:

    sla b
    dec a
    jr nz,set_bit_reg1

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
    ld hl,NBYTES_INT-1
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
;     HL: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF BC' DE' HL' AF'
add_reg_to_acc:

    ld de,acc
    jr add_reg2_to_reg1

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
;   Afeta: BC DE HL AF

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

    push ix
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
    dec bc

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
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
stack_test:

    push ix
    ld ix,0xFFF8    ; Reserva 8 bytes
    add ix,sp
    ld sp,ix

    ld c,(ix+0)
    ld b,(ix+1)
    ;....
    ld hl,0x0008    ; Libera 8 bytes
    add hl,sp
    ld sp,hl
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
;   shr_reg
;   void shr_reg(uint8_t *reg, int places);
;   Parâmetros:
;     DE: reg
;     BC: places
;   Retorna: Nada
;   Afeta: BC DE HL AF
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

    push ix
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
    dec bc

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
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
;   mul_reg2_by_reg1
;   void mul_reg2_by_reg1(const uint8_t *reg1, uint8_t *reg2);
;   Parâmetros:
;     HL: reg1
;     DE: reg2
;   Retorna: Nada
;   Afeta: BC DE HL AF IY BC' DE' HL' AF'
mul_reg2_by_reg1:

    push ix
    ld ix,0xFFF8-2*NBYTES    ; Reserva 8 bytes + 2 buffers
    add ix,sp
    ld sp,ix

    ld (ix+0),l
    ld (ix+1),h         ; IX+0, IX+1: reg1
    ld (ix+2),e
    ld (ix+3),d         ; IX+2, IX+3: reg2

;    uint8_t regmdiv[NBYTES];
    ld c,ixl
    ld b,ixh            ; Copia IX em BC
    ld HL,8
    add hl,bc
    ld (ix+4),l
    ld (ix+5),h         ; IX+4, IX+5: regmdiv

;    uint8_t regmdiv2[NBYTES];
    ld bc,NBYTES
    add hl,bc
    ld (ix+6),l
    ld (ix+7),h         ; IX+6, IX+7: regmdiv2

;    memcpy(regmdiv, reg2, NBYTES);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld bc,NBYTES
    ldir

;    memcpy(regmdiv2, reg2, NBYTES);
    ld e,(ix+6)
    ld d,(ix+7)         ; IX+6, IX+7: regmdiv2
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld bc,NBYTES
    ldir

;    zero_reg(reg2);
    ld l,(ix+2)
    ld h,(ix+3)
    call zero_reg       ;Zera reg2

;    int places = 0;
    ld iy,0             ; IY = places

;    // Parte inteira
;    for (int i = NBYTES_INT-1; i>=0; i--){

    ld bc,NBYTES_INT
    ld l,(ix+0)
    ld h,(ix+1)       ; IX+0, IX+1: reg1
    add hl,bc
    dec hl            ; HL: buffer a processar

mul_reg2_by_reg1_1:

;        int msk = 1;
    ld d,1

;        for (int j = 0; j < 8; j++){
    ld e,8

mul_reg2_by_reg1_2:

;            if (reg1[i] & msk){
    ld a,(hl)
    and d
    jr z, mul_reg2_by_reg1_3

;                shl_reg(regmdiv,places);
    push bc
    push de
    push hl

    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld b,iyh
    ld c,iyl            ; BC recebe IY = 'places'
    call shl_reg

;                add_reg2_to_reg1(reg2, regmdiv);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regmdiv
    call add_reg2_to_reg1

    pop hl
    pop de
    pop bc

;                places = 1;
    ld iy,1     ; IY = places
;            }
;            else
;                places++;
    jr mul_reg2_by_reg1_4

mul_reg2_by_reg1_3:

    inc iy

mul_reg2_by_reg1_4:

;            msk <<= 1;
    ld a,d
    sla a
    ld d,a

;        }
    dec e
    jr nz,mul_reg2_by_reg1_2

;    }
    dec hl
    dec bc
    ld a,b
    or c
    jr nz,mul_reg2_by_reg1_1

;    memcpy(regmdiv, regmdiv2, NBYTES);
    ld l,(ix+6)
    ld h,(ix+7)         ; IX+6, IX+7: regmdiv2
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld bc,NBYTES
    ldir

;    shr_reg(regmdiv,1);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld bc,1
    call shr_reg

;    places = 0;
    ld iy,0

;    //Parte fracionária
;    for (int i = NBYTES_INT; i < NBYTES; i++){
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg1
    ld bc,NBYTES_INT
    add hl,bc
    ld bc, NBYTES - NBYTES_INT

mul_reg2_by_reg1_5:

;        int msk = 128;
    ld d,128

;        for (int j = 0; j < 8; j++){
    ld e,8

mul_reg2_by_reg1_6:

;            if (reg1[i] & msk){
    ld a,(hl)
    and d
    jr z,mul_reg2_by_reg1_7

    push bc
    push de
    push hl

;                shr_reg(regmdiv,places);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld b,iyh
    ld c,iyl            ; BC recebe IY = 'places'
    call shr_reg

;                add_reg2_to_reg1(reg2, regmdiv);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regmdiv
    call add_reg2_to_reg1

    pop hl
    pop de
    pop bc

;                places = 1;
    ld iy,1
    jr mul_reg2_by_reg1_8
;            }
;            else
;                places++;
mul_reg2_by_reg1_7:

    inc iy

mul_reg2_by_reg1_8:

;            msk >>= 1;
    ld a,d
    srl a
    ld d,a

;        }
    dec e
    jr nz, mul_reg2_by_reg1_6

;    }
    inc hl
    dec bc
    ld a,b
    or c
    jr nz,mul_reg2_by_reg1_5

mul_reg2_by_reg1_end:

    ld hl,0x0008+2*NBYTES    ; Libera 8 bytes + 2 buffers
    add hl,sp
    ld sp,hl
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
;   mul_acc_by_reg
;   void mul_acc_by_reg(const uint8_t *reg);
;   Parâmetros:
;     HL: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF IY BC' DE' HL' AF'
mul_acc_by_reg:

;    mul_reg2_by_reg1(reg, acc);
    ld de,acc
    call mul_reg2_by_reg1
    ret

;///////////////////////////////////////////////////////////////////////////////
;   mul_reg_10
;   void mul_reg_10(uint8_t *reg);
;   Parâmetros:
;     DE: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF IY BC' DE' HL' AF'
mul_reg_10:

    push ix
    ld ix,0xFFF8-NBYTES    ; Reserva 8 bytes + buffer
    add ix,sp
    ld sp,ix

    ld (ix+0),e
    ld (ix+1),d         ; IX+0, IX+1: reg

;    uint8_t regmdiv[NBYTES];
    ld c,ixl
    ld b,ixh            ; Copia IX em BC
    ld HL,8
    add hl,bc
    ld (ix+4),l
    ld (ix+5),h         ; IX+4, IX+5: regmdiv

;    memcpy(regmdiv, reg, NBYTES);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg
    ld bc,NBYTES
    ldir

;    shl_reg(reg, 1);
    ld e,(ix+0)
    ld d,(ix+1)         ; IX+0, IX+1: reg
    ld bc,1
    call shl_reg

;    shl_reg(regmdiv, 3);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld bc,3
    call shl_reg

;    add_reg2_to_reg1(reg,regmdiv);
    ld e,(ix+0)
    ld d,(ix+1)         ; IX+0, IX+1: reg
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regmdiv
    call add_reg2_to_reg1

    ld hl,0x0008+NBYTES    ; Libera 8 bytes + buffer
    add hl,sp
    ld sp,hl
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
;   compare
;   int compare (const uint8_t *reg1, const uint8_t *reg2);
;   Parâmetros:
;     HL: reg1
;     DE: reg2
;   Retorna: Flags: Z:        reg1 = reg2
;                   NZ e C:   reg1 < reg2
;                   NC e NC:  reg1 > reg2
;   Afeta: BC DE HL AF
compare:

;    for (int i = 0; i < NBYTES; i++){
    ld bc,NBYTES

compare_1:

;        if (reg1[i] < reg2[i]) return -1;   // reg1 < reg2
;        if (reg1[i] > reg2[i]) return 1;    // reg1 > reg2
;    }

    ld a,(de)
    cpi
    jr nz,compare_2
    inc de
    jp pe,compare_1

;    return 0;   // Iguais
    xor a
    ret

compare_2:
    dec hl
    cp (hl)
    ccf
    ret

;///////////////////////////////////////////////////////////////////////////////
;   iszero
;   int iszero (const uint8_t *reg);
;   Parâmetros:
;     HL: reg
;   Retorna: Flags: Z:        reg = 0
;                   NZ:       reg1 != 0
;   Afeta: BC DE HL AF
iszero:

;    for (int i = 0; i < NBYTES; i++){
    ld bc,NBYTES
    xor a

iszero_1:

;        if (reg[i]) return 0;   // não é zero
    cpi
    ret nz

;    }
    jp pe,iszero_1

;    return 1;   // É zero
    xor a
    ret

;///////////////////////////////////////////////////////////////////////////////
;   div_reg2_by_reg1
;   int div_reg2_by_reg1(const uint8_t *reg1, uint8_t *reg2);
;   Parâmetros:
;     HL: reg1
;     DE: reg2
;   Retorna: Flags: Z:        Divisão OK
;                   NZ:       Divisão com erro
;   Afeta: BC DE HL AF IY BC' DE' HL' AF'
div_reg2_by_reg1:

    push ix
    ld ix,0xFFF6-3*NBYTES    ; Reserva 10 bytes + 3 buffers
    add ix,sp
    ld sp,ix

    ld (ix+0),l
    ld (ix+1),h         ; IX+0, IX+1: reg1
    ld (ix+2),e
    ld (ix+3),d         ; IX+2, IX+3: reg2

;    uint8_t regmdiv[NBYTES];
    ld c,ixl
    ld b,ixh            ; Copia IX em BC
    ld HL,10
    add hl,bc
    ld (ix+4),l
    ld (ix+5),h         ; IX+4, IX+5: regmdiv

;    uint8_t regmdiv2[NBYTES];
    ld bc,NBYTES
    add hl,bc
    ld (ix+6),l
    ld (ix+7),h         ; IX+6, IX+7: regmdiv2

;    if (iszero(reg1)) return -1; // Divisão por zero
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg
    call iszero
    jr nz, div_reg2_by_reg1_1
    ld a,0xff
    jp div_reg2_by_reg1_end

div_reg2_by_reg1_1:

;    int res = compare (reg2, reg1);
;    if (res == 0){   // iguais
;        zero_reg(reg2);
;        set_bit_reg_int(reg2, 0);    //1
;        return 0;   //ok
;    }
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg1
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    call compare
    jr nz, div_reg2_by_reg1_2

    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    call zero_reg
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld bc,0
    call set_bit_reg_int
    xor a
    jp div_reg2_by_reg1_end

div_reg2_by_reg1_2:

    ;call prints
    ;db "Vai dividir\n",0

;    uint8_t regquot[NBYTES];
    ld bc,NBYTES
    ld l,(ix+6)
    ld h,(ix+7)         ; IX+6, IX+7: regmdiv2
    add hl,bc
    ld (ix+8),l
    ld (ix+9),h         ; IX+8, IX+9: regquot

;    zero_reg(regquot);
    ;ld l,(ix+8)
    ;ld h,(ix+9)         ; IX+8, IX+9: regquot
    call zero_reg

;    memcpy(regmdiv, reg1, NBYTES);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg1
    ld bc,NBYTES
    ldir

;    memcpy(regmdiv2, reg1, NBYTES);
    ld e,(ix+6)
    ld d,(ix+7)         ; IX+6, IX+7: regmdiv2
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg1
    ld bc,NBYTES
    ldir

    ;call prints
    ;db "(1) ",0

;    for (;!iszero(reg2);){
div_reg2_by_reg1_3:

    ;call prints
    ;db "(1_3) ",0

    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    call iszero
    jp z, div_reg2_by_reg1_5

;        int res = compare (reg2, regmdiv);
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    call compare

;        if (res == -1){ // reg2 < regmdiv
    jp nc, div_reg2_by_reg1_4
    jp z, div_reg2_by_reg1_4

;            int order = 0;
    ld iy,0

;            for (;!iszero(reg2);){
div_reg2_by_reg1_3a:

    ;call prints
    ;db "(1_3a) ",0

    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    call iszero
    jr z, div_reg2_by_reg1_3b

    ;call prints
    ;db "(1_3aa) ",0

;                if (compare(reg2, regmdiv) < 0){ //reg2 < regmdiv

    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    call compare
    jr nc, div_reg2_by_reg1_3c
    jr z, div_reg2_by_reg1_3c

    ;call prints
    ;db "shr_reg(regmdiv,1) ",0

;                    shr_reg(regmdiv, 1);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld bc,1
    call shr_reg

;                    order++;
    inc iy
;                    if (order >= NBITS_FRAC){
;                        memcpy(reg2, regquot, NBYTES);
;                        return 0;
;                    }
    ld d,iyh
    ld e,iyl
    ex de,hl
    ld de,NBITS_FRAC
    xor a
    sbc hl,de
    jr nc, div_reg2_by_reg1_3b

;                   continue;
    ;call prints
    ;db "(1_3a A) ",0
    jr div_reg2_by_reg1_3a
;                }

div_reg2_by_reg1_3c:

    ;call prints
    ;db "(1_3a C) ",0

;                sub_reg2_from_reg1(reg2, regmdiv);
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+0, IX+1: regmdiv
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    call sub_reg2_from_reg1

;                set_bit_reg(regquot, NBITS_FRAC - order);
    ld hl,NBITS_FRAC
    ld c,iyl
    ld b,iyh
    xor a
    sbc hl,bc
    ld c,l
    ld b,h
    ld l,(ix+8)
    ld h,(ix+9)         ; IX+8, IX+9: regquot
    call set_bit_reg

;            }
    ;call prints
    ;db "(1_3a B) ",0
    jp div_reg2_by_reg1_3a

div_reg2_by_reg1_3b:

    ;call prints
    ;db "(==1_3b) ",0

;            memcpy(reg2, regquot, NBYTES);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    ld l,(ix+8)
    ld h,(ix+9)         ; IX+8, IX+9: regquot
    ld bc,NBYTES
    ldir

;            return 0;
    xor a
    jp div_reg2_by_reg1_end

;        }
;        else{   // reg2 >= regmdiv

div_reg2_by_reg1_4:

;            int order = 0;
    ld iy,0

;            for (;;){
div_reg2_by_reg1_4a:

    ;call prints
    ;db "(1_4a) ",0

;                if (compare(reg2, regmdiv) < 0){ //reg2 < regmdiv
;                    break;
;                }
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    call compare
    jr c, div_reg2_by_reg1_4b

;                shl_reg(regmdiv, 1);
    ld bc,1
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    call shl_reg

;                if (compare(reg2, regmdiv) < 0){ //reg2 < regmdiv
;                    break;
;                }
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: reg2
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    call compare
    jr c, div_reg2_by_reg1_4b


;                order++;
    inc iy

;            }
    jr div_reg2_by_reg1_4a

div_reg2_by_reg1_4b:

;            memcpy(regmdiv, regmdiv2, NBYTES);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld l,(ix+6)
    ld h,(ix+7)         ; IX+6, IX+7: regmdiv2
    ld bc,NBYTES
    ldir

;            shl_reg(regmdiv, order);
    ld c,iyl
    ld b,iyh
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    call shl_reg

;            sub_reg2_from_reg1(reg2, regmdiv);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regmdiv
    call sub_reg2_from_reg1

;            set_bit_reg_int(regquot, order);
    ld l,(ix+8)
    ld h,(ix+9)         ; IX+8, IX+9: regquot
    ld c,iyl
    ld b,iyh
    call set_bit_reg_int

;            memcpy(regmdiv, regmdiv2, NBYTES);
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regmdiv
    ld l,(ix+6)
    ld h,(ix+7)         ; IX+6, IX+7: regmdiv2
    ld bc,NBYTES
    ldir

;        }
;    }
    jp div_reg2_by_reg1_3

div_reg2_by_reg1_5:

;    memcpy(reg2, regquot, NBYTES);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: reg2
    ld l,(ix+8)
    ld h,(ix+9)         ; IX+8, IX+9: regquot
    ld bc,NBYTES
    ldir

;    return 0;
    xor a

div_reg2_by_reg1_end:

    ld hl,0x000A+3*NBYTES    ; Libera 10 bytes + 3 buffers
    add hl,sp
    ld sp,hl
    pop ix
    or a
    ret

;///////////////////////////////////////////////////////////////////////////////
;   div_acc_by_reg
;   int div_acc_by_reg(const uint8_t *reg)
;   Parâmetros:
;     HL: reg
;   Retorna: Flags: Z:        Divisão OK
;                   NZ:       Divisão com erro
;   Afeta: BC DE HL AF IY BC' DE' HL' AF'
div_acc_by_reg:

    ;return div_reg2_by_reg1(reg, acc);
    ld de,acc
    jp div_reg2_by_reg1

;///////////////////////////////////////////////////////////////////////////////
;   print_int_part
;   void print_int_part(const uint8_t *reg);
;   Parâmetros:
;     HL: reg
;   Retorna: Nada
;   Afeta: BC DE HL AF
print_int_part_digit:

    ld e,0
    xor a

print_int_part_digit_0:

    sbc hl,bc
    jr z,print_int_part_digit_2b
    jr c,print_int_part_digit_2a
    inc e
    jr print_int_part_digit_0

print_int_part_digit_2a:
    add hl,bc
    jr print_int_part_digit_2

print_int_part_digit_2b:
    inc e

print_int_part_digit_2:

    ld a,e
    or a
    jr nz,print_int_part_digit_3

    bit 0,d
    ret z

print_int_part_digit_3:

    ld d,1
    add a,'0'
    out (0),a

    ret

print_int_part:

    ld b,(hl)
    inc hl
    ld c,(hl)
    ld l,c
    ld h,b

    ld d,0
    ld bc,10000
    call print_int_part_digit
    ld bc,1000
    call print_int_part_digit
    ld bc,100
    call print_int_part_digit
    ld bc,10
    call print_int_part_digit
    ;ld bc,1
    ;call print_int_part_digit
    ld a,l
    add a,'0'
    out (0),a
    ret

;///////////////////////////////////////////////////////////////////////////////
;   print_reg_decimal
;   void print_reg_decimal(const uint8_t *reg, int nplaces);
;   Parâmetros:
;     HL: reg
;     BC: nplaces
;   Retorna: Nada
;   Afeta: BC DE HL AF
print_reg_decimal:

    push ix
    ld ix,0xFFF8-NBYTES    ; Reserva 8 bytes + buffer
    add ix,sp
    ld sp,ix
    ld (ix+0),l
    ld (ix+1),h         ; IX+0, IX+1: reg

    ;uint8_t regtmp[NBYTES];
    ld e,ixl
    ld d,ixh            ; Copia IX em DE
    ld HL,8
    add hl,de
    ld (ix+2),l
    ld (ix+3),h         ; IX+2, IX+3: regtmp

;    print_int_part(reg);
;    printf(".");
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: reg
    push hl
    push bc
    call print_int_part
    pop bc
    ld a,'.'
    out (0),a
    pop hl

;    memcpy(regtmp, reg, NBYTES);
    push bc

    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: regtmp
    ld bc,NBYTES
    ldir

    pop bc

    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: regtmp

;    if (nplaces)
;        ++nplaces;
    ld a,b
    or c
    jr z, print_reg_decimal_1
    inc bc

print_reg_decimal_1:

;    for (;;){
;        if (nplaces){
    ld a,b
    or c
    jr z, print_reg_decimal_2

;            --nplaces;
    dec bc
;            if (!nplaces)
;                break;
    ld a,b
    or c
    jr z, print_reg_decimal_3
;        }

print_reg_decimal_2:

;        memset(regtmp,0,NBYTES_INT);
    xor a
    ld (hl),a
    inc hl
    ld (hl),a
    dec hl

;        if (iszero(regtmp)) break;
    push bc
    push hl
    call iszero
    pop hl
    pop bc
    jr z, print_reg_decimal_3

;        mul_reg_10(regtmp);
    push bc
    push hl
    ld e,l
    ld d,h
    call mul_reg_10
    pop hl
    pop bc

;        print_int_part(regtmp);
    inc hl
    ld a,(hl)
    dec hl
    add a,'0'
    out (0),a

;    }
    jr print_reg_decimal_1

print_reg_decimal_3:
print_reg_decimal_end:

    ld hl,0x0008+NBYTES    ; Libera 8 bytes + buffer
    add hl,sp
    ld sp,hl
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
;   println_reg_decimal
;   void println_reg_decimal(const uint8_t *reg, int nplaces);
;   Parâmetros:
;     HL: reg
;     BC: nplaces
;   Retorna: Nada
;   Afeta: BC DE HL AF
println_reg_decimal:

    call print_reg_decimal
    call print_crlf
    ret

;///////////////////////////////////////////////////////////////////////////////
;   test_pi_bbp
;   void test_pi_bbp(void); // Bailey-Borwein-Plouffe
;   Parâmetros: Nada
;   Retorna: Nada
;   Afeta: BC DE HL AF
test_pi_bbp:

    push ix
    ld ix,0xFFF6-3*NBYTES    ; Reserva 10 bytes + 3 buffers
    add ix,sp
    ld sp,ix

;    uint8_t regtotal[NBYTES];
    ld c,ixl
    ld b,ixh            ; Copia IX em BC
    ld HL,10
    add hl,bc
    ld (ix+0),l
    ld (ix+1),h         ; IX+0, IX+1: regtotal

;    uint8_t regsubtotal[NBYTES];
    ld bc,NBYTES
    add hl,bc
    ld (ix+2),l
    ld (ix+3),h         ; IX+2, IX+3: regsubtotal

;    uint8_t regden[NBYTES];
    ;ld bc,NBYTES
    add hl,bc
    ld (ix+4),l
    ld (ix+5),h         ; IX+4, IX+5: regden

;    zero_reg(regtotal);
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: regtotal
    call zero_reg

;    for (int k = 0; k < 10000; k++){
    ld bc,0

test_pi_bbp_1:

    ld (ix+6),c
    ld (ix+7),b         ; IX+6, IX+7: k

    ld a,b
    cp NUM_IT >> 8
    jr c, test_pi_bbp_1a

    ;call prints
    ;db " T1 ",0

    ld a,c
    cp NUM_IT & 0xFF
    jr c, test_pi_bbp_1a

    ;call prints
    ;db " T2 ",0

    jp test_pi_bbp_2

test_pi_bbp_1a:

    ;call prints
    ;db " GO ",0


;        zero_reg(regsubtotal);
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: regsubtotal
    call zero_reg

;        zero_reg(regden);
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regden
    push hl
    call zero_reg
    pop de              ; DE contém regden

;        load_reg_int(regden,8*k+1);     // Inicia com 8k+1
    ld l,(ix+6)
    ld h,(ix+7)         ; IX+6, IX+7: k
    sla l
    rl h
    sla l
    rl h
    sla l
    rl h
    inc hl
    call load_reg_int   ; DE já deve estar certo aqui

;        zero_reg(acc);
    ld hl,acc
    call zero_reg

;        set_bit_reg_int(acc, 2);        // Inicia com 4
    ld hl,acc
    ld bc,2
    call set_bit_reg_int

;        div_acc_by_reg(regden);
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regden
    call div_acc_by_reg

;        add_reg2_to_reg1(regsubtotal,acc);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: regsubtotal
    ld hl,acc
    call add_reg2_to_reg1

;        inc_reg_int8(regden, 3);        // 8k+4
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regden
    ld a,3
    call inc_reg_int8

;        zero_reg(acc);
    ld hl,acc
    call zero_reg

;        set_bit_reg_int(acc, 1);        // Inicia com 2
    ld hl,acc
    ld bc,1
    call set_bit_reg_int

;        div_acc_by_reg(regden);
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regden
    call div_acc_by_reg

;        sub_reg2_from_reg1(regsubtotal,acc);
    ld hl,acc
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: regsubtotal
    call sub_reg2_from_reg1

;        inc_reg_int8(regden, 1);        // 8k+5
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regden
    ld a,1
    call inc_reg_int8

;        zero_reg(acc);
    ld hl,acc
    call zero_reg

;        set_bit_reg_int(acc, 0);        // Inicia com 1
    ld hl,acc
    ld bc,0
    call set_bit_reg_int

;        div_acc_by_reg(regden);
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regden
    call div_acc_by_reg

;        sub_reg2_from_reg1(regsubtotal,acc);
    ld hl,acc
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: regsubtotal
    call sub_reg2_from_reg1

;        inc_reg_int8(regden, 1);        // 8k+6
    ld e,(ix+4)
    ld d,(ix+5)         ; IX+4, IX+5: regden
    ld a,1
    call inc_reg_int8

;        zero_reg(acc);
    ld hl,acc
    call zero_reg

;        set_bit_reg_int(acc, 0);        // Inicia com 1
    ld hl,acc
    ld bc,0
    call set_bit_reg_int

;        div_acc_by_reg(regden);
    ld l,(ix+4)
    ld h,(ix+5)         ; IX+4, IX+5: regden
    call div_acc_by_reg

;        sub_reg2_from_reg1(regsubtotal,acc);
    ld hl,acc
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: regsubtotal
    call sub_reg2_from_reg1

;        shr_reg(regsubtotal, 4*k);
    ld e,(ix+2)
    ld d,(ix+3)         ; IX+2, IX+3: regsubtotal
    ld c,(ix+6)
    ld b,(ix+7)         ; IX+6, IX+7: k
    sla c
    rl b
    sla c
    rl b
    call shr_reg

;        add_reg2_to_reg1(regtotal, regsubtotal);
    ld e,(ix+0)
    ld d,(ix+1)         ; IX+0, IX+1: regtotal
    ld l,(ix+2)
    ld h,(ix+3)         ; IX+2, IX+3: regsubtotal
    call add_reg2_to_reg1

;;        if (!(k % 10)){
;;            printf("%5d: ",k);
;;            print_reg_decimal(regtotal, 120);
;;            int places_ok = compare_digits_to_pi(regtotal);
;;            printf(" (%d)\n",places_ok);
;;            if (places_ok >= 1000) return;
;;        }
;            print_reg_decimal(regtotal, 120);

    call prints
    db "TOTAL:",0
    ld l,(ix+0)
    ld h,(ix+1)         ; IX+0, IX+1: regtotal
    ld bc,NUM_DECS
    call println_reg_decimal
    ;call prints
    ;db "LOOP",0

;    }
    ld c,(ix+6)
    ld b,(ix+7)         ; IX+6, IX+7: k
    inc bc
    ;ld (ix+6),c
    ;ld (ix+7),b         ; IX+6, IX+7: k

    jp test_pi_bbp_1

test_pi_bbp_2:
test_pi_bbp_end:

    ld hl,0x000A+3*NBYTES    ; Libera 10 bytes + 3 buffers
    add hl,sp
    ld sp,hl
    pop ix
    ret

;///////////////////////////////////////////////////////////////////////////////
;   _main
;   void _main(void);
;   Parâmetros: Nada
;   Retorna: Nada
;   Afeta: BC DE HL AF IY BC' DE' HL' AF'
_main:

    ; dividendo
    ;ld hl,acc
    ;call zero_reg
    ;ld hl,acc
    ;ld bc,1
    ;call set_bit_reg_int
    ;ld hl,acc
    ;ld bc,3
    ;call set_bit_reg_int
    ;ld hl,acc
    ;ld bc,8
    ;call set_bit_reg_int

    ; divisor
    ;ld hl,reg1
    ;call zero_reg
    ;ld hl,reg1
    ;ld bc,0
    ;call set_bit_reg_int
    ;ld hl,reg1
    ;ld bc,1
    ;call set_bit_reg_int

    ; resultado em acc
    ;ld de,acc
    ;ld hl,reg1
    ;call div_reg2_by_reg1

    ;call prints
    ;db 13,10,"Result:",0
    ;ld hl,acc
    ;ld bc,100
    ;call println_reg_decimal

    call test_pi_bbp

    ret

;///////////////////////////////////////////////////////////////////////////////
    seek RAMBASE
    org RAMBASE

reg1:   ds NBYTES
acc:    ds NBYTES

