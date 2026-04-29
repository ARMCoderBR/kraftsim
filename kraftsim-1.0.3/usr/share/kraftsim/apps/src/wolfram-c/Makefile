ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				wolfram.bin

../io-kraft80/crt0-kraft80-loadable.rel:
				cd ../io-kraft80; make; cd -

../io-kraft80/io-kraft80.rel:
				cd ../io-kraft80; make; cd -

wolfram.bin:			wolfram.c ../io-kraft80/io-kraft80.h ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel 
				sdcc $(COPTS) wolfram.c
				sdcc -mz80 --code-loc 0x2000 --data-loc 0x4000 --no-std-crt0 ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel wolfram.rel
				mv crt0-kraft80-loadable.ihx wolfram.ihx
				objcopy --input-target=ihex --output-target=binary wolfram.ihx wolfram.bin
				cp wolfram.bin ../../wolfram.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

