ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				clock2.bin

../io-kraft80/crt0-kraft80-loadable.rel:
				cd ../io-kraft80; make; cd -

../io-kraft80/io-kraft80.rel:
				cd ../io-kraft80; make; cd -

clock2.bin:			clock2.c videodisp.c videodisp.h ../io-kraft80/io-kraft80.h ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel 
				sdcc $(COPTS) clock2.c
				sdcc $(COPTS) videodisp.c
				sdcc -mz80 --code-loc 0x2000 --data-loc 0x3000 -DUSE_KRAFTMON=0 --no-std-crt0 ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel clock2.rel videodisp.rel
				mv crt0-kraft80-loadable.ihx clock2.ihx
				objcopy --input-target=ihex --output-target=binary clock2.ihx clock2.bin
				cp clock2.bin ../../clock2.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

