ASMOPTS=-o -l -s -g -I../io-kraft80

all:				invaders.bin

invaders.rel:			invaders.s defines.inc
				sdasz80 $(ASMOPTS) invaders.s

graphics.rel:			graphics.s defines.inc
				sdasz80 $(ASMOPTS) graphics.s

inputs.rel:			inputs.s defines.inc
				sdasz80 $(ASMOPTS) inputs.s

timer.rel:			timer.s defines.inc
				sdasz80 $(ASMOPTS) timer.s

font.rel:			font.s defines.inc
				sdasz80 $(ASMOPTS) font.s

sound.rel:			sound.s defines.inc
				sdasz80 $(ASMOPTS) sound.s

invaders.bin:			invaders.rel graphics.rel inputs.rel timer.rel font.rel sound.rel
				sdcc -mz80 --no-std-crt0 --code-loc 0x2000 --data-loc 0x4400 invaders.rel graphics.rel inputs.rel timer.rel font.rel sound.rel
				objcopy --input-target=ihex --output-target=binary invaders.ihx invaders.bin

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

