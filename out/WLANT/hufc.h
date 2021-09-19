#ifndef HUFC_H
#define HUFC_H

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>


#define NUM_HUFFMAN_TREES 7
#define NUM_MODULATION_TYPES 5
#define NUM_SYMBOLS 48


/* 
Compression mode:
 0 - selection
 1 - Always Huffman 
 2 - Always RLE 
*/
void encode(unsigned char* src, int srcLen, unsigned char* dest, int* destLenBytes, int* destLenBits, int compressionMode);
void encodeRLE(unsigned char* src, int srcLen, unsigned char* dest, int* destLenBytes, int* destLenBits, int BitsValue, int BitsLength);

int decode(unsigned char* src, int srcLen, unsigned char* dest);
int decodeRLE(unsigned char* src, int srcLen, unsigned char* dest, int BitsValue);




#endif