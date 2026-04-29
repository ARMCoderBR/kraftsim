ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				pibbp-asm.bin

pibbp-asm.bin:			pibbp.s
				sdasz80 $(ASMOPTS) pibbp.s
				sdcc -mz80 --no-std-crt0 --code-loc 0x2000 pibbp.rel
				mv pibbp.ihx pibbp-asm.ihx
				objcopy --input-target=ihex --output-target=binary pibbp-asm.ihx pibbpasm.bin
				cp pibbpasm.bin ../../pibbpasm.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

