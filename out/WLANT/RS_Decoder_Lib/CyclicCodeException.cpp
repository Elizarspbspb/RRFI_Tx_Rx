/** Exception class for cyclic code classes implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 06.11.2003
 */

# ifndef     CYCLICCODEEXCEPTION_H
#   include "CyclicCodeException.h"
# endif

#include <stdio.h>
#include <string.h>
#include <memory.h>

// --------------------------------------------------------------------------

CCyclicCodeException::CCyclicCodeException(int iExceptionCode)
: m_iExceptionCode (iExceptionCode) 
{
}

// --------------------------------------------------------------------------

void CCyclicCodeException::toString(char *strExceptionInfo) {
  toString(m_iExceptionCode, strExceptionInfo);
}

// --------------------------------------------------------------------------

void CCyclicCodeException::toString(int iExceptionCode, char *strExceptionInfo) {
  static char* s_aCodeToInfoMapping[] = {
    (char *)exceptionMemoryAllocationError,            "Memory allocation error",
    (char *)exceptionCodeAlreadyInitialized,           "Code is already initialized",
    (char *)exceptionMatrixReductionError,             "Matrix has linearly dependent lines, but reduction was suppressed",
    (char *)exceptionCodeIsNotInitialized,             "Code is not initialized"
  };
  
  strExceptionInfo[0] = 0;
  
  for (int i = 0; i < sizeof(s_aCodeToInfoMapping) / sizeof(char *); i += 2) {
//    if (iExceptionCode == (int)s_aCodeToInfoMapping[i]) {
    if ((char*)iExceptionCode == s_aCodeToInfoMapping[i]) {
      memcpy(strExceptionInfo, s_aCodeToInfoMapping[i + 1], strlen(s_aCodeToInfoMapping[i + 1]) + 1);
      break;
    }
  }

  if (strExceptionInfo[0] == 0) {
    sprintf(strExceptionInfo, "(exception code %d)", iExceptionCode);
  }
}

// --------------------------------------------------------------------------

int  CCyclicCodeException::getCode() {
  return m_iExceptionCode;
}

// --------------------------------------------------------------------------
