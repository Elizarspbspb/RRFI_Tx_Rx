/** LDPC encode used parity check matrix header file
 *
 * @authors Andrey Efimov
 * @organization: LCA, ISS Dept, SUAI
 * $Revision: $
 * $Date: $
 */

#include "ldpc_encode.h"
#include "ldpc_code.h"

extern ldpc_code ldpc;
/**
 * Codes message vector
 * @param pMessage pointer to message vector, size [LDPCMessageLength]
 * @param pCodeword pointer to codeword made, size [LDPCCodewordLength]
 * @return 
 *  0: no error
 */ 
int ldpc_encode(const char *pMessage, char *pCodeword) {
  int i, j;
  int iPos;
  unsigned int val;

  iPos = 0;
  for(j = 0; j < LDPCMessageLength; j++) 
    pCodeword[ldpc.MessageSet[j]] = pMessage[j];

  for(i = 0; i < NCE; i++) {
    val = 0;

    for (j = 0; j < LDPCMessageLength; j++) {
      if (pMessage[j]) {
        val ^= ldpc.ElHComp[i * LDPCCodewordLength + ldpc.MessageSet[j]];
      }
    }

    for (j = 0; j < 8 * sizeof(unsigned int); j++) {
      if (iPos >= LDPCCodewordLength - LDPCMessageLength)
        break;
      pCodeword[ldpc.CheckSet[iPos++]] = val & 1;
      val >>= 1;
    }
  }

  return 0;
}

// --------------------------------------------------------------------------

