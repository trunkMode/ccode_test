/*--------------------------------------------------------
base64.c
This file copyright 1997-2000, SonicWALL, Inc.

This is for base64 encoding and decoding.  See below for
detailed description.

Creation Date: 3/31/00
Creator:
 
$Author: bruce $
$Revision: 1.1 $
$Date: 2000/03/31 22:18:01 $

 (From RFC1521 and draft-ietf-dnssec-secext-03.txt)
   The following encoding technique is taken from RFC 1521 by Borenstein
   and Freed.  It is reproduced here in a slightly edited form for
   convenience.

   A 65-character subset of US-ASCII is used, enabling 6 bits to be
   represented per printable character. (The extra 65th character, "=",
   is used to signify a special processing function.)

   The encoding process represents 24-bit groups of input bits as output
   strings of 4 encoded characters. Proceeding from left to right, a
   24-bit input group is formed by concatenating 3 8-bit input groups.
   These 24 bits are then treated as 4 concatenated 6-bit groups, each
   of which is translated into a single digit in the base64 alphabet.

   Each 6-bit group is used as an index into an array of 64 printable
   characters. The character referenced by the index is placed in the
   output string.

                         Table 1: The Base64 Alphabet

      Value Encoding  Value Encoding  Value Encoding  Value Encoding
          0 A            17 R            34 i            51 z
          1 B            18 S            35 j            52 0
          2 C            19 T            36 k            53 1
          3 D            20 U            37 l            54 2
          4 E            21 V            38 m            55 3
          5 F            22 W            39 n            56 4
          6 G            23 X            40 o            57 5
          7 H            24 Y            41 p            58 6
          8 I            25 Z            42 q            59 7
          9 J            26 a            43 r            60 8
         10 K            27 b            44 s            61 9
         11 L            28 c            45 t            62 +
         12 M            29 d            46 u            63 /
         13 N            30 e            47 v
         14 O            31 f            48 w         (pad) =
         15 P            32 g            49 x
         16 Q            33 h            50 y

   Special processing is performed if fewer than 24 bits are available
   at the end of the data being encoded.  A full encoding quantum is
   always completed at the end of a quantity.  When fewer than 24 input
   bits are available in an input group, zero bits are added (on the
   right) to form an integral number of 6-bit groups.  Padding at the
   end of the data is performed using the '=' character.

   Since all base64 input is an integral number of octets, only the
         -------------------------------------------------                       
   following cases can arise:
   
       (1) the final quantum of encoding input is an integral
           multiple of 24 bits; here, the final unit of encoded
           output will be an integral multiple of 4 characters
           with no "=" padding,
       (2) the final quantum of encoding input is exactly 8 bits;
           here, the final unit of encoded output will be two
           characters followed by two "=" padding characters, or
       (3) the final quantum of encoding input is exactly 16 bits;
           here, the final unit of encoded output will be three
           characters followed by one "=" padding character.

----------------------------------------------------------*/

/*--------------------------------------------------------*/
/* Included files
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define FALSE       0
#define TRUE        1
/*--------------------------------------------------------*/
/* defined constants and macros
 */


/*--------------------------------------------------------*/
/* structures and typedefs
 */


/*--------------------------------------------------------*/
/* global variables
 */

static const char Base64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';

/* This table is indexed by an ASCII character and gives the equivalent
 * base64 value, or 64 if there is no corresponding base64 value */
const int pr2six[256]={
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0x00 - 0x0F */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0x10 - 0x1F */
	64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,	/* ASCII 0x20 - 0x2F */
	52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,	/* ASCII 0x30 - 0x3F */
	64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,	/* ASCII 0x40 - 0x4F */
	15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,	/* ASCII 0x50 - 0x5F */
	64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,	/* ASCII 0x60 - 0x6F */
	41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,	/* ASCII 0x70 - 0x7F */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0x80 - 0x8F */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0x90 - 0x9F */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0xA0 - 0xAF */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0xB0 - 0xBF */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0xC0 - 0xCF */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0xD0 - 0xDF */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,	/* ASCII 0xE0 - 0xEF */
	64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64		/* ASCII 0xF0 - 0xFF */
};

/*--------------------------------------------------------*/
/* private prototypes
 */

/*--------------------------------------------------------*/
/* code
 */
char *b64encode(char *src, int srclength)
{
    size_t datalength = 0;
    unsigned char input[3];
    unsigned char output[4];
    int i;
	unsigned int targsize;
	char *target;

	/* round up to a multiple of 3, expand by 4/3, and add
	 * one for the null terminating byte
	 */
	targsize = (((srclength+2)/3) * 4) + 1;
	target = (char *)malloc(targsize);
	if (!target)
		return 0;

	while (2 < srclength) {
		input[0] = *src++;
        input[1] = *src++;
        input[2] = *src++;
        srclength -= 3;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
        output[3] = input[2] & 0x3f;

        if (datalength + 4 > targsize)
		{
			free(target);
			printf("Error in encoding 0\n");
                        return 0;
		}
        target[datalength++] = Base64[output[0]];
        target[datalength++] = Base64[output[1]];
        target[datalength++] = Base64[output[2]];
        target[datalength++] = Base64[output[3]];
    }
    
    /* Now we worry about padding. */
    if (0 != srclength) {
        /* Get what's left. */
        input[0] = input[1] = input[2] = '\0';
        for (i = 0; i < srclength; i++)
            input[i] = *src++;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);

        if (datalength + 4 > targsize)
		{
			free(target);
			printf("Error in encoding 1\n");
		    return 0;
		}
        target[datalength++] = Base64[output[0]];
        target[datalength++] = Base64[output[1]];
        if (srclength == 1)
                target[datalength++] = Pad64;
        else
                target[datalength++] = Base64[output[2]];
        target[datalength++] = Pad64;
    }
    if (datalength >= targsize)
	{
		free(target);
		printf("Error in encoding 2\n");
                return 0;
	}
    target[datalength] = '\0';      /* Returned value doesn't count \0. */
    return (target);
}

/*
 * Encode:
 * Split input into 6-bit words and convert those from base64 to ASCII
 * (output will be expanded to 4/3 of the input size)
 */
char *base64encode(char *src)
{
	return b64encode(src, strlen(src));
}


char *b64decode(char *bufCoded, int *pnOutLen, int inPlace)
{
	int nBytesDecoded;
	unsigned char *bufIn;
	char *bufPlain;
	unsigned char *bufOut;
	int nPrBytes;

	while (*bufCoded == ' ' || *bufCoded == '\t')
		bufCoded++;

	bufIn = (unsigned char*)bufCoded;
	while (pr2six[*(bufIn++)] <= 63);
	nPrBytes = (char*)bufIn - bufCoded - 1;
	nBytesDecoded = ((nPrBytes+3)/4) * 3;

	if (inPlace)
	{
		bufPlain = bufCoded;
	}
	else
	{
		bufPlain = (char*)malloc(nBytesDecoded+1);
		if (!bufPlain)
			return 0;
	}

	bufOut = (unsigned char*)bufPlain;
	bufIn = (unsigned char*)bufCoded;

	while (nPrBytes > 0)
	{
		*(bufOut++) = (unsigned char)(pr2six[*bufIn] << 2 
			| pr2six[bufIn[1]] >> 4);
		*(bufOut++) = (unsigned char)(pr2six[bufIn[1]] << 4 
			| pr2six[bufIn[2]] >> 2);
		*(bufOut++) = (unsigned char)(pr2six[bufIn[2]] << 6 
			| pr2six[bufIn[3]]);
		bufIn += 4;
		nPrBytes -= 4;
	}

	if (nPrBytes & 03)
	{
		if (pr2six[bufIn[-2]] > 63)
			nBytesDecoded -= 2;
		else
			nBytesDecoded -= 1;
	}
	bufPlain[nBytesDecoded] = '\0';
	if (pnOutLen)
		*pnOutLen = nBytesDecoded;
	return bufPlain;
}

/*
 * Decode:
 * Inverse of encode - convert ASCII characters to 6-bit base64 equivalents,
 * and pack those together to form 3 bytes from every 4 characters.
 * (output will be packed down to 3/4 of the input size)
 */
char *base64decode(char *bufCoded)
{
	return b64decode(bufCoded, NULL, FALSE);
}

/*
 * Decode in-place:
 * This is like base64decode(), but does the decode in-place in the
 * source buffer - we can do this on a decode because the decoded data
 * is smaller than the encoded source data (3/4 the size of it).
 */
void base64decodeInplace(char *bufCoded)
{
	b64decode(bufCoded, NULL, TRUE);
}



/*--------------------------------------------------------*/

