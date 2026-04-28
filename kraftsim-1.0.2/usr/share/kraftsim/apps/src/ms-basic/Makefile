all:				bas32k.ihx

bas32k.ihx:			bas32k.s
				sdasz80 -o -l -s -g bas32k.s
				sdcc --verbose -mz80 --no-std-crt0 -Wl -b_CODE=0x2000 bas32k.rel
				objcopy --input-target=ihex --output-target=binary bas32k.ihx bas32k.bin
				cp bas32k.bin ../../msbasic.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

