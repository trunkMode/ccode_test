/*
 * @func: read raw data 
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "signed_data.bin.c"
int main()
{
    FILE *fp = fopen("data.bin", "w");

    fwrite(data, 1, sizeof(data), fp);
    fclose(fp);
    return 0;
}
