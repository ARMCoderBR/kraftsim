/*
WOLFRAM.C
Example program for the KRAFT 80
2025 - ARM Coder
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "io-kraft80.h"

#pragma codeseg CODE

////////////////////////////////////////////////////////////////////////////////
int wolfram(int width, int height, int rulenum) {

    char cells[160];
    char nextcells[160];

    memset(cells, 0, width);
    memset(nextcells, 0, width);

    cells[width / 2] = 1;

    char rule[8];

    int mask = 1;

    ////////////////////////////////////////////////////////////////////////////

    int i;

    for (i = 0; i < 8; i++) {

        if (rulenum & mask)
            rule[i] = 1;
        else
            rule[i] = 0;

        mask <<= 1;
    }

    ////////////////////////////////////////////////////////////////////////////

    printf("\n.");
    for (i = 0; i < width; i++)
        putchar('=');
    printf(".\n.");

    int it;
    for (it = 0; it < height; it++) {

        for (i = 0; i < width; i++) {

            if (cells[i])
                putchar('#');
            else
                putchar(' ');
        }

        printf(".\n.");

        for (i = 0; i < width; i++) {

            int il = i - 1;
            if (il < 0)
                il = width - 1;

            int ir = i + 1;
            if (ir == width)
                ir = 0;

            int idx = 4 * cells[il] + 2 * cells[i] + cells[ir];

            nextcells[i] = rule[idx];
        }

        memcpy(cells, nextcells, width);
    }

    for (i = 0; i < width; i++)
        putchar('=');

    printf(".\n");

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]){

char buf[16];
    
    printf("\nWolfram Cell Automaton 1.0 by ARMCoder - 2025,26\n");

    for (;;){

        printf("\nEnter rule (0-255), empty to exit:");
        lgets(buf, sizeof(buf));

        if (!buf[0]) break;

        int rule = atoi(buf);
        
        wolfram(50, 20, rule);
    }

    return 0;
}

