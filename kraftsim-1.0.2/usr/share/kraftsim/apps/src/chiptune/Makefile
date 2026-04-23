ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				chiptune.bin

chiptune.bin:			chiptune.s
				sdasz80 $(ASMOPTS) chiptune.s
				sdcc -mz80 --no-std-crt0 --code-loc 0x2000 chiptune.rel
				objcopy --input-target=ihex --output-target=binary chiptune.ihx chiptune.bin
				cp chiptune.bin ../../chiptune.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

