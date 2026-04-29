/*
TEST.C
Example program for the KRAFT 80
2026 - ARM Coder
*/

#include <stdio.h>
#include <ctype.h>

#include "io-kraft80.h"
#include "kraft80.h"

#pragma codeseg CODE


////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]) __sdcccall(0){

    int c=300;
    int j = argc;
    printf("argc:%d\r\n",j);

    int i;

    for (i = 0; i < j; i++)
    	printf("argv[%d]:%s\n",i,argv[i]);

    return 0;
}

