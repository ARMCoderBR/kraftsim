ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				chiptune2.bin vgm

chiptune2.bin:			chiptune2.s challenger.s
				sdasz80 $(ASMOPTS) chiptune2.s
				sdcc -mz80 --no-std-crt0 --code-loc 0x2000 chiptune2.rel
				objcopy --input-target=ihex --output-target=binary chiptune2.ihx chiptu2.bin
				cp chiptu2.bin ../../chiptu2.com

vgm:				vgm.c
				gcc -c vgm.c
				gcc -o vgm vgm.o

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin *.o vgm

