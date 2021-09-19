 /** Galois Classes Exception implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 02.10.2003
 */

#include <string.h>
#include <stdio.h>

# ifndef     GALOISEXCEPTION_H
#   include "GaloisException.h"
# endif

// --------------------------------------------------------------------------

CGaloisException::CGaloisException(int iGaloisExceptionCode)
: m_iGaloisExceptionCode (iGaloisExceptionCode) 
{
}

// --------------------------------------------------------------------------

void CGaloisException::toString(char *strGaloisExceptionInfo) {
  toString(m_iGaloisExceptionCode, strGaloisExceptionInfo);
}

// --------------------------------------------------------------------------

void CGaloisException::toString(int iGaloisExceptionCode, char *strGaloisExceptionInfo) {
  static char* s_aCodeToInfoMapping[] = {
    (char *)exceptionDivisionByZero,                   "Division by zero"                      ,
    (char *)exceptionGenerativePolynomialNotPrimitive, "Generative polynomial is not primitive",
    (char *)exceptionMemoryAllocationError,            "Memory allocation error"
  };
  
  strGaloisExceptionInfo[0] = 0;
  
  for (int i = 0; i < sizeof(s_aCodeToInfoMapping) / sizeof(char *); i += 2) {
//    if (iGaloisExceptionCode == (int)s_aCodeToInfoMapping[i]) {
    if ((char*)iGaloisExceptionCode == s_aCodeToInfoMapping[i]) {
      memcpy(strGaloisExceptionInfo, s_aCodeToInfoMapping[i + 1], strlen(s_aCodeToInfoMapping[i + 1]) + 1);
      break;
    }
  }

  if (strGaloisExceptionInfo[0] == 0) {
    sprintf(strGaloisExceptionInfo, "(exception code %d)", iGaloisExceptionCode);
  }
}

// --------------------------------------------------------------------------

int  CGaloisException::getCode() {
  return m_iGaloisExceptionCode;
}

// --------------------------------------------------------------------------
