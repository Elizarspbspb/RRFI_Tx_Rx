# ifndef   CONV_CODE_H
#   define CONV_CODE_H

#define ConvCodeGenPolySize 7

struct convCode
{
   unsigned char ConvCodeGenPoly1[ConvCodeGenPolySize]; // First generator polynom
   unsigned char ConvCodeGenPoly2[ConvCodeGenPolySize]; // Second generator polynom
   unsigned prev_state[2][64]; // Trellis tables for Viterbi decoding
   int prev_state_outbits[4][64];
};
extern struct convCode conv_code;

#endif