all:		rom1-picalc.ihx

rom1-picalc.ihx:	rom1-picalc.s
		sdasz80 -o -l -s -g rom1-picalc.s
		sdcc -mz80 --no-std-crt0 rom1-picalc.rel

clean:
		rm *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex

