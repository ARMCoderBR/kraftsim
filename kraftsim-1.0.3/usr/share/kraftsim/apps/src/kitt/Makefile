ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				kitt.bin blinker.bin

kitt.bin:			kitt.s
				sdasz80 $(ASMOPTS) kitt.s
				sdcc -mz80 --no-std-crt0 --code-loc 0x2000 kitt.rel
				objcopy --input-target=ihex --output-target=binary kitt.ihx kitt.bin
				cp kitt.bin ../../kitt.com

blinker.bin:			blinker.s
				sdasz80 $(ASMOPTS) blinker.s
				sdcc -mz80 --no-std-crt0 --code-loc 0x2000 blinker.rel
				objcopy --input-target=ihex --output-target=binary blinker.ihx blinker.bin
				cp blinker.bin ../../blinker.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

