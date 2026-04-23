#include <stdint.h>
#include <stdio.h>

const uint16_t table[15] = { 0b0000110011000111, 
			     0b1010111001100100,
			     0b1011010000110111,
			     0b1011011101101101,
			     0b1111101100111010,
			     0b1101010111001101,
			     0b1111101111101110,
			     0b0101000011010011,
			     0b1111001110000000,
			     0b0000001110101100,
			     0b0111011110001110,
			     0b0011100111010001,
			     0b1101010101011010,
			     0b1101010100000000,
			     0b1110010010110111 };
			     
uint16_t func(uint16_t in, uint8_t key){

    uint16_t aux = in;

    aux ^= table[key & 15];

    return aux;
}

unsigned lfsr_fib(void)
{
    uint16_t start_state = 0xACE1u;  /* Any nonzero start state will work. */
    uint16_t lfsr = start_state;
    uint16_t bit;                    /* Must be 16-bit to allow bit<<15 later in the code */
    unsigned period = 0;
    uint8_t key = 0;
    uint16_t lfsr_out;
    
    do
    {   /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1u;
        lfsr = (lfsr >> 1) | (bit << 15);
        ++period;
        
        lfsr_out = func(lfsr,key);
	printf("%016b %016b\n",lfsr,lfsr_out);
	key++; if (key == 15) key = 0;
    }
    while (lfsr != start_state);

    return period;
}

int main (int argc, char *argv[]){

    unsigned int period = lfsr_fib();
 
    printf("Period:%u\n",period);  
   
    return 0;
}

