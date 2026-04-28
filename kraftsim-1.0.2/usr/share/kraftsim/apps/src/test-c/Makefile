all:				test.bin

../io-kraft80/crt0-kraft80-loadable.rel:
				cd ../io-kraft80; make; cd -

../io-kraft80/io-kraft80.rel:
				cd ../io-kraft80; make; cd -

test.bin:			test.c ../io-kraft80/io-kraft80.h ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel 
				sdcc -c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99 test.c
				sdcc -mz80 --code-loc 0x2000 --data-loc 0x5000 -DUSE_KRAFTMON=0 --no-std-crt0 ../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel test.rel  
				mv crt0-kraft80-loadable.ihx test.ihx
				objcopy --input-target=ihex --output-target=binary test.ihx test.bin
				cp test.bin ../../test.com

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin

