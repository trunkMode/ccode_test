/*
 * * Test: SPARC/Solaris 8 64-bit kernel mode
 * * gcc -Wall -pipe -g -o bus bus.c
 * */
#include <stdio.h>
#include <stdlib.h>

int main ( int argc, char * argv[] )
{
    unsigned int        i = 0x12345678;
    unsigned short int *q = NULL;
    unsigned char      *p = ( unsigned char * )&i;

    *p = 0x00;
    q  = ( unsigned short int * )( p + 1 );
    *q = 0x0000;
    return( EXIT_SUCCESS );
}
