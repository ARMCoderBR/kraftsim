ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				pibbp-c.bin

pibbp-c.bin:			pibbp.c ../io-kraft80/io-kraft80.h ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel 
				sdcc $(COPTS) pibbp.c
				sdcc -mz80 --code-loc 0x2000 --data-loc 0x4000 --no-std-crt0 ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel pibbp.rel
				mv crt0-kraft80-loadable.ihx pibbp-c.ihx
				objcopy --input-target=ihex --output-target=binary pibbp-c.ihx pibbp-c.bin

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

