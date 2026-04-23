ASMOPTS=-o -l -s -g -I../io-kraft80
COPTS=-c -V --disable-warning 85 -mz80 -I../io-kraft80 --opt-code-size --std-sdcc99

all:				krapman.bin gensprite

../io-kraft80/crt0-kraft80-loadable.rel:
				cd ../io-kraft80; make; cd -

../io-kraft80/io-kraft80.rel:
				cd ../io-kraft80; make; cd -

krapman.rel:			krapman.c *.h ../io-kraft80/io-kraft80.h
				sdcc $(COPTS) krapman.c

map.rel:			map.c *.h ../io-kraft80/io-kraft80.h
				sdcc $(COPTS) map.c

graphics.rel:			graphics.c *.h ../io-kraft80/io-kraft80.h
				sdcc $(COPTS) graphics.c

game.rel:			game.c *.h ../io-kraft80/io-kraft80.h
				sdcc $(COPTS) game.c

graphres.rel:			graphres.c *.h
				sdcc $(COPTS) graphres.c

inputs.rel:			inputs.s ../io-kraft80/kraft80.inc
				sdasz80 $(ASMOPTS) inputs.s

timer.rel:			timer.s ../io-kraft80/kraft80.inc
				sdasz80 $(ASMOPTS) timer.s

krapman.bin:			krapman.rel graphics.rel map.rel game.rel graphres.rel inputs.rel timer.rel \
					../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel 
				sdcc -mz80 --code-loc 0x2000 --data-loc 0x8000 --no-std-crt0 \
					../io-kraft80/crt0-kraft80-loadable.rel ../io-kraft80/io-kraft80.rel krapman.rel \
					graphres.rel map.rel graphics.rel inputs.rel timer.rel game.rel
				mv crt0-kraft80-loadable.ihx krapman.ihx
				objcopy --input-target=ihex --output-target=binary krapman.ihx krapman.bin
				cp krapman.bin ../../krapman.com

gensprite:			gensprite.c
				gcc -c gensprite.c
				gcc -o gensprite gensprite.o

clean:
				rm -f *.ihx *.map *.sym *.lk *.rel *.lst *.noi *.asm *.mem *.hex *.bin *.o

