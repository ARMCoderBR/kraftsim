all:		bas32k-standalone.ihx

bas32k-standalone.ihx:	bas32k-standalone.s
		sdasz80 -o -l -s -g bas32k-standalone.s
		#sdcc -mz80 --no-std-crt0 -Wl -b_CODE=0x0000,-b_DATA=0x2000 picalc.rel
		sdcc --verbose -mz80 --no-std-crt0 bas32k-standalone.rel

clean:
		rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex

