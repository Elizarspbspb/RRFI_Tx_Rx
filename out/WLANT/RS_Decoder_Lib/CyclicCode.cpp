    /** Base class for any cyclic code implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dem_pT@SUAI
 * @date: 06.11.2003
 */

#include "CyclicCode.h"
#include "CyclicCodeException.h"
#include "GaloisFieldInterface.h"
#include "GaloisExtendedField.h"
#include "GaloisExtendedField2mFast.h"
#include "GaloisPolynomial.h"
#include <algorithm>

#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <math.h>

/*
template <class T>
void printmatrixf(T *mx, int width, int height, const char *fmt = "%d ", const char *filename = "matrix.txt", const char *filemethod = "w") {
  FILE *out = fopen(filename, filemethod);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      fprintf(out, fmt, mx[i * width + j]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  fclose(out);
}

template <class T>
void printvectorf(char *info, T *mx, int width, const char *fmt = "%d ", const char *filename = "matrix.txt", const char *filemethod = "w") {
  FILE *out = fopen(filename, filemethod);
  fprintf(out, "%s", info);
  fprintf(out, " (%d)", width);
  for (int j = 0; j < width; j++) {
    fprintf(out, fmt, mx[j]);
  }
  fprintf(out, "\n");
  fclose(out);
}
*/



// --------------------------------------------------------------------------

CCyclicCode::CCyclicCode() 
: m_bInitialized (false)
, m_bDecoderInitialized (false)
, m_iN (0)
, m_iK (0)
, m_iR (0)
, m_pG (NULL)
, m_pH (NULL)
, m_pGF (NULL)
, m_bGFAutoDelete (false)
, m_iMinDist (-1)
, m_iOffset (1)
, m_pSyndromes (NULL)
, m_pSigma (NULL)
, m_pB (NULL)
, m_pT (NULL)
, m_pRoots (NULL)
, m_pGenPoly (NULL)
, m_pMessageSet (NULL)
, m_pDecodedWord (NULL)
, m_pDecodedWordBest (NULL)
, m_pThresholds (NULL)
, m_pErasures (NULL)
, m_pErasuresPositions (NULL)
, m_pReliabilities (NULL)
, m_pHardDecision (NULL)
{
}

// --------------------------------------------------------------------------

CCyclicCode::~CCyclicCode() {
  if (m_pG)
    delete [] m_pG;
  if (m_pH)
    delete [] m_pH;
  if (m_bGFAutoDelete && m_pGF)
    delete [] m_pGF;
  if (m_pSyndromes)
    delete [] m_pSyndromes;
  if (m_pSigma)
    delete [] m_pSigma;
  if (m_pB)
    delete [] m_pB;
  if (m_pT)
    delete [] m_pT;
  if (m_pRoots)
    delete [] m_pRoots;
  if (m_pGenPoly)
    delete [] m_pGenPoly;
  if (m_pMessageSet)
    delete [] m_pMessageSet;
  if (m_pDecodedWordBest)
    delete [] m_pDecodedWordBest;
  if (m_pDecodedWord)
    delete [] m_pDecodedWord;
  if (m_pThresholds)
    delete [] m_pThresholds;
  if (m_pErasures)
    delete [] m_pErasures;
  if (m_pErasuresPositions)
    delete [] m_pErasuresPositions;
  if (m_pReliabilities)
    delete [] m_pReliabilities;
  if (m_pHardDecision)
    delete [] m_pHardDecision;
}

// --------------------------------------------------------------------------

void CCyclicCode::initFromH(value_type *H, int iR, int iN, IGaloisField *pGF, int iMinDist /* = -1*/, IGaloisField *pGFExt /*= NULL*/, int iOffset /* = 1*/, bool bReduce /* = false */) {
  int i, j;
  int iLine, iCol;
  int m_pTr, m_pTr1, m_pTr2;
  char *HI_Flag = NULL;
  value_type div_val, tmp;

  if (m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeAlreadyInitialized);

  m_iMinDist = iMinDist;
  m_pGFRoots = pGFExt;
  m_iOffset = iOffset;
  
  m_pH = new value_type[iR * iN];
  if (!m_pH)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_iN = iN;
  m_iR = iR;
  m_pGF = pGF;
  memcpy(m_pH, H, m_iR * m_iN * sizeof(value_type));

  if (m_pGFRoots)
    allocateDecoderBuffers();

  try {
    HI_Flag = new char[m_iN];
    if (!HI_Flag)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
    memset(HI_Flag, 0, m_iN);

    iLine = 0;
    iCol = 0;
    // iCol = iN - iR;
    // printmatrixf(m_pH, m_iN, m_iR, 1, "matr_H_constr.txt", "w");
    while (iCol < iN && iLine < iR) {
      if (m_pH[iLine * iN + iCol] == 0) {
        // ищем среди строк, которые находятся ниже данной, строку, в которой в столбце iCol стоит единица
        for (i = iLine + 1; i < iR; i++) {
          if (m_pH[i * iN + iCol] != 0)
            break;
        }
        if (i < iR) {
          // есть строка, содержащая не 0 в данном столбце 
          // переставляем строки с номерами i и iLine
          for(j = 0; j < iN; j++) {
            tmp = m_pH[i * iN + j];
            m_pH[i * iN + j] = m_pH[iLine * iN + j];
            m_pH[iLine * iN + j] = tmp;
          }
        }
        else {
          // строки с не 0 нет
          iCol++;
        }
      }
      else {
        // вычитаем текущую строку из всех, в которых содержится не 0 в столбце iCol, так, чтобы стал 0
        for (i = 0; i < iR; i++) {
          if (i != iLine && m_pH[i * iN + iCol] != 0) {
            div_val = m_pGF->div(m_pH[i * iN + iCol], m_pH[iLine * iN + iCol]);
            for (j = 0; j < iN; j++) {
              m_pH[i * iN + j] = m_pGF->sub(m_pH[i * iN + j], m_pGF->mul(m_pH[iLine * iN + j], div_val));
            }
          }
        }
        div_val = m_pH[iLine * iN + iCol];
        for (j = 0; j < iN; j++) {
          m_pH[iLine * iN + j] = m_pGF->div(m_pH[iLine * iN + j], div_val);
        }
        HI_Flag[iCol] = 1;
        iLine++;
        iCol++;
      }
      // printmatrixf(m_pH, m_iN, m_iR, 1, "matr_H_constr.txt", "a");
    }

    // строки, которые находятся ниже строки iLine - нулевые
    if (iLine < iR && !bReduce)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMatrixReductionError);
    
    m_iR = iLine;
    m_iK = m_iN - m_iR;

    m_pMessageSet = new value_type[m_iK];
    if (!m_pMessageSet)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

    // make generative matrix
    m_pG = new value_type[m_iK * m_iN];
    if (!m_pG)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
    memset(m_pG, 0, m_iK * m_iN * sizeof(value_type));
    
    m_pTr = 0;
    m_pTr1 = 0;
    for (i = 0; i < m_iN; i++) {
      if (HI_Flag[i] == 0) {
        for(j = 0, m_pTr2 = 0; j < m_iN; j++) {
          if (HI_Flag[j] != 0) 
            m_pG[m_pTr * m_iN + j] = m_pGF->sub(0, m_pH[m_pTr2++ * m_iN + i]);
        }
        m_pG[m_pTr * m_iN + i] = 1;
        m_pMessageSet[m_pTr++] = i;
      }
      else {
        m_pTr1++;
      }
    }

  }
  catch (CCyclicCodeException) {
    if (HI_Flag)
      delete [] HI_Flag;

    throw;
  }
  
  delete [] HI_Flag;

  m_bInitialized = true;
  
//  printmatrixf(m_pH, m_iN, m_iR, "%3d ", "matrixH.txt");
//  printmatrixf(m_pG, m_iN, m_iK, "%3d ", "matrixG.txt");
}

// --------------------------------------------------------------------------

void CCyclicCode::initFromG(value_type *G, int iK, int iN, IGaloisField *pGF, int iMinDist /* = -1*/, IGaloisField *pGFExt /* = NULL*/, int iOffset /* = 1*/, bool bReduce /* = false */) {
  int i, j;
  int iLine, iCol;
  int m_pTr, m_pTr1, m_pTr2;
  char *HI_Flag = NULL;
  value_type div_val, tmp;

  if (m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeAlreadyInitialized);

  m_pG = new value_type[iK * iN];
  if (!m_pG)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_iMinDist = iMinDist;
  m_pGFRoots = pGFExt;
  m_iOffset = iOffset;
  
  m_iN = iN;
  m_iK = iK;
  m_pGF = pGF;
  memcpy(m_pG, G, m_iK * m_iN * sizeof(value_type));

  if (m_pGFRoots)
    allocateDecoderBuffers();

  try {
    HI_Flag = new char[m_iN];
    if (!HI_Flag)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
    memset(HI_Flag, 0, m_iN);

    iLine = 0;
    iCol = 0;
    while (iCol < iN && iLine < iK) {
      if (m_pG[iLine * iN + iCol] == 0) {
        // ищем среди строк, которые находятся ниже данной, строку, в которой в столбце iCol стоит единица
        for (i = iLine + 1; i < iK; i++) {
          if (m_pG[i * iN + iCol] != 0)
            break;
        }
        if (i < iK) {
          // есть строка, содержащая не 0 в данном столбце 
          // переставляем строки с номерами i и iLine
          for(j = 0; j < iN; j++) {
            tmp = m_pG[i * iN + j];
            m_pG[i * iN + j] = m_pG[iLine * iN + j];
            m_pG[iLine * iN + j] = tmp;
          }
        }
        else {
          // строки с не 0 нет
          iCol++;
        }
      }
      else {
        // вычитаем текущую строку из всех, в которых содержится не 0 в столбце iCol, так, чтобы стал 0
        for (i = 0; i < iK; i++) {
          if (i != iLine && m_pG[i * iN + iCol] != 0) {
            div_val = m_pGF->div(m_pG[i * iN + iCol], m_pG[iLine * iN + iCol]);
            for (j = 0; j < iN; j++) {
              m_pG[i * iN + j] = m_pGF->sub(m_pG[i * iN + j], m_pGF->mul(m_pG[iLine * iN + j], div_val));
            }
          }
        }
        div_val = m_pG[iLine * iN + iCol];
        for (j = 0; j < iN; j++) {
          m_pG[iLine * iN + j] = m_pGF->div(m_pG[iLine * iN + j], div_val);
        }
        HI_Flag[iCol] = 1;
        iLine++;
        iCol++;
      }
    }

    // строки, которые находятся ниже строки iLine - нулевые
    if (iLine < iK && !bReduce)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMatrixReductionError);
    
    m_iK = iLine;
    m_iR = m_iN - m_iK;

    m_pMessageSet = new value_type[m_iK];
    if (!m_pMessageSet)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

    // make generative matrix
    m_pH = new value_type[m_iR * m_iN];
    if (!m_pH)
      throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
    memset(m_pH, 0, m_iR * m_iN * sizeof(value_type));
    
    m_pTr = 0;
    m_pTr1 = 0;
    for (i = 0; i < m_iN; i++) {
      if (HI_Flag[i] == 0) {
        for(j = 0, m_pTr2 = 0; j < m_iN; j++) {
          if (HI_Flag[j] != 0) 
            m_pH[m_pTr * m_iN + j] = m_pGF->sub(0, m_pG[m_pTr2++ * m_iN + i]);
        }
        m_pH[m_pTr * m_iN + i] = 1;
        m_pTr++;
      }
      else {
        m_pMessageSet[m_pTr1++] = i;
      }
    }

  }
  catch (CCyclicCodeException) {
    if (HI_Flag)
      delete [] HI_Flag;

    throw;
  }

  delete [] HI_Flag;

  m_bInitialized = true;

//  printmatrixf(m_pH, m_iN, m_iR, "%3d ", "matrixH.txt");
//  printmatrixf(m_pG, m_iN, m_iK, "%3d ", "matrixG.txt");

#pragma message("Падает при вызове initFromGFile()")
}

// --------------------------------------------------------------------------

void CCyclicCode::initRSCode(IGaloisField *pGF, int iMinDist, int iNShortened, int iOffset /* = 1*/) {
  int i, j;
  int iR, iN, iNFull;
  if (m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeAlreadyInitialized);

  iR = iMinDist - 1;
  iNFull = pGF->getCardinality() - 1;
  iN = iNShortened;
  
  // creates parity check matrix of RS code
  value_type *H = new value_type[iR * iN];
  if (!H)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  const int iPrimitive = 2;
  for (j = 0; j < iR; j++) {
    for (i = 0; i < iN; i++) {
      H[j * iN + i] = pGF->pow(pGF->pow(iPrimitive, i + iNFull - iN), j + iOffset);
    }
  }

//  printmatrixf(H, iNShortened, iR, "%3d ", "matrixH_nr.txt");

  try {
    initFromH(H, iR, iNShortened, pGF, iR + 1, pGF, iOffset, true);
  }
  catch (CCyclicCodeException) {
    delete [] H;
    throw;
  }

  delete [] H;
}

// --------------------------------------------------------------------------

void CCyclicCode::initRSCodePoly(IGaloisField *pGF, int iMinDist, int iNShortened, int iOffset /*= 1*/) {
  int i, j;
  value_type val;
  const int iPrimitive = 2;
  
  if (m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeAlreadyInitialized);

  // m_iN = pGF->getCardinality() - 1;
  m_iN = iNShortened;
  m_iR = iMinDist - 1;
  m_iK = m_iN - m_iR;

  m_iMinDist = iMinDist;
  m_pGFRoots = pGF;
  m_iOffset = iOffset;
  m_pGF = pGF;

  m_pGenPoly = new value_type[m_iR + 2];
  memset(m_pGenPoly, 0, (m_iR + 2) * sizeof(value_type));

  m_pGenPoly[0] = 1;
  for (i = 0; i < m_iR; i++) {
    memmove(m_pGenPoly + 1, m_pGenPoly, (i + 2) * sizeof(value_type));
    m_pGenPoly[0] = 0;
    val = pGF->pow(iPrimitive, i + iOffset);
    for (j = 0; j <= i; j++) 
      m_pGenPoly[j] = pGF->sub(m_pGenPoly[j], pGF->mul(val, m_pGenPoly[j + 1]));
  }

//  printvectorf("gp: ", m_pGenPoly, m_iR + 1, "%3d ", "gen_poly.txt");

  m_pG = new value_type[m_iK * m_iN];
  if (!m_pG)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
  memset(m_pG, 0, m_iK * m_iN * sizeof(value_type));

  for (j = 0; j < m_iK; j++) {
    for (i = 0; i <= m_iR; i++) {
      m_pG[j * m_iN + j + i] = m_pGenPoly[i];
    }
  }

  m_pH = NULL;
  #pragma message("initRSCodePoly - матрица H не проинициализирована.")

  if (m_pGFRoots)
    allocateDecoderBuffers();

//  printmatrixf(m_pG, m_iN, m_iK, "%3d ", "matrixG.txt");

  m_bInitialized = true;
}

// --------------------------------------------------------------------------

void CCyclicCode::initBCHfromRSCode(IGaloisField *pGFRS, int iRSMinDist, int iOffset /* = 1*/) {
  int i, j, k;
  int iR, iN;
  CGaloisPolynomial* pPoly;
  const int iPrimitive = 2;

  if (m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeAlreadyInitialized);

  iR = iRSMinDist - 1;
  iN = pGFRS->getCardinality() - 1;
  
  // creates parity check matrix of RS code
  value_type *H = new value_type[iR * pGFRS->getExtension() * iN];
  if (!H)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  for (j = 0; j < iR; j++) {
    for (i = 0; i < iN; i++) {
      int iRSPow = pGFRS->pow(pGFRS->pow(iPrimitive, i), j + iOffset);
      pPoly = pGFRS->getElementPoly(iRSPow);
      for (k = 0; k < pGFRS->getExtension(); k++) {
        H[(j * pGFRS->getExtension() + k) * iN + i] = pPoly->getCoefficient(k);
      }
    }
  }

//  printmatrixf(H, iN, iR * pGFRS->getExtension(), "%3d ", "matrixH_nr.txt");

  try {
    initFromH(H, iR * pGFRS->getExtension(), iN, pGFRS->getBase(), -1, pGFRS, iOffset, true);
    m_iMinDist = m_iR + 1;
  }
  catch (CCyclicCodeException) {
    delete [] H;
    throw;
  }

  delete [] H;
}

// --------------------------------------------------------------------------

void CCyclicCode::initFromGFile(const char *sFileName, int iK, int iN, IGaloisField *pGF, int iMinDist /* = -1*/, IGaloisField *pGFExt /* = NULL*/, int iOffset /* = 1*/, bool bReduce /* = false*/) {
  int i, j, val;

  if (m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeAlreadyInitialized);

  value_type *G = new value_type[iK * iN];
  if (!G)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  FILE *in = fopen(sFileName, "r");
  if (!in)
    printf("error opening file. insert exception\n");
  
  for (i = 0; i < iK; i++) {
    for (j = 0; j < iN; j++) {
      fscanf(in, "%d", &val);
      G[i * iN + j] = val;
    }
  }

  fclose(in);

  try {
    initFromG(G, iK, iN, pGF, iMinDist, pGFExt, iOffset, bReduce);
  }
  catch (CCyclicCodeException) {
    delete [] G;
    throw;
  }
  
  delete [] G;
}

// --------------------------------------------------------------------------

int CCyclicCode::getCodewordLength() {
  return m_iN;
}

// --------------------------------------------------------------------------

int CCyclicCode::getMessageLength() {
  return m_iK;
}

// --------------------------------------------------------------------------

int CCyclicCode::getMinDistance() {
  return m_iMinDist;
}

// --------------------------------------------------------------------------

void CCyclicCode::encode(value_type *pMessage, value_type* pCodeword) {
  int i, j;

  if (!m_bInitialized)
    throw CCyclicCodeException(CCyclicCodeException::exceptionCodeIsNotInitialized);

  memset(pCodeword, 0, m_iN * sizeof(value_type));
  
  for (i = 0; i < m_iK; i++) {
    if (pMessage[i] != 0) {
      for (j = 0; j < m_iN; j++) {
        pCodeword[j] = m_pGF->sum(pCodeword[j], m_pGF->mul(pMessage[i], m_pG[i * m_iN + j]));
      }
    }
  }
/**/
/*
  // a(x) = g(x) * m(x)
  for (i = 0; i <= m_iR; i++) {
    for (j = 0; j < m_iK; j++) {
      pCodeword[i + j] = m_pGF->sum(pCodeword[i + j], m_pGF->mul(pMessage[j], m_pGenPoly[i]));
    }
  }
/**/
}

// --------------------------------------------------------------------------

void CCyclicCode::encode(const unsigned char *pMessage, unsigned char *pCodeword) {
  int i, j;

  for (i = 0; i < m_iK; i++) {
    m_pB[i] = 0;
    for (j = 0; j < m_pGF->getExtension(); j++) {
      m_pB[i] |= (value_type)pMessage[i * m_pGF->getExtension() + j] << j;
    }
  }

  encode(m_pB, m_pT);

  for (i = 0; i < m_iN; i++) {
    for (j = 0; j < m_pGF->getExtension(); j++) {
      pCodeword[i * m_pGF->getExtension() + j] = (m_pT[i] >> j) & 1;
    }
  }
}

// --------------------------------------------------------------------------

// if pReliabilities[i] < 0 then hard disicion bit is 0, if pReliabilities[i] > 0 - 1
void CCyclicCode::decode_soft(const float *pReliabilities, unsigned char *pMessage) {
  int i, j;
  float fProbMax = 0;
  
  for (i = 0; i < m_iN * m_pGF->getExtension(); i++) {
    if (fProbMax < fabs(pReliabilities[i]))
      fProbMax = fabs(pReliabilities[i]);
  }

  for (i = 0; i < m_iN; i++) {
    m_pReliabilities[i] = 1;
    m_pHardDecision[i] = 0;
    for (j = 0; j < m_pGF->getExtension(); j++) {
      m_pReliabilities[i] *= fabs(pReliabilities[i * m_pGF->getExtension() + j]) / fProbMax;
      m_pHardDecision[i] |= ((pReliabilities[i * m_pGF->getExtension() + j] < 0) ? 0 : 1) << j;
    }
  }
  
  decode_soft(m_pHardDecision, m_pReliabilities, NULL, NULL);
  
  for (i = 0; i < m_iK; i++) {
    for (j = 0; j < m_pGF->getExtension(); j++) {
      pMessage[i * m_pGF->getExtension() + j] = (m_pHardDecision[m_pMessageSet[i]] >> j) & 1;
    }
  }

}

// --------------------------------------------------------------------------

bool CCyclicCode::decode(value_type* pVector, value_type* pMessage /* = NULL*/, value_type* pCodeword /* = NULL */, int iNumOfErasures /* = 0*/, int *pErasureLocations /*= NULL*/) {
  int i, j;
  int iDegree;
  value_type val, tmp, tmp2;

  if (pCodeword) {
    memcpy(pCodeword, pVector, m_iN * sizeof(value_type));
    pVector = pCodeword;
  }

  // evaluate syndromes
  if (!calculateSyndromes(pVector) || iNumOfErasures > 0) {
    // setup register
    memset(m_pSigma, 0, m_iN * sizeof(value_type));
    m_pSigma[0] = 1;
    for (i = 0; i < iNumOfErasures; i++) {
      val = m_pGFRoots->sub(0, pErasureLocations[i/* - 1*/] + 1);
      
      tmp2 = 1;
      for (j = 1; j <= i + 1; j++) {
        tmp = m_pSigma[j];
        m_pSigma[j] = m_pGFRoots->sum(m_pSigma[j], m_pGFRoots->mul(val, tmp2));
        tmp2 = tmp;
      }
    }

//    printvectorf("erasures: ", m_pSigma, iNumOfErasures + 1, "%3d ", "erasures.txt");

    // execute Berlekamp-Massey algorithm
    iDegree = BerlekampMassey(iNumOfErasures);
    if (iDegree < 0) 
      return false; // decoding failed

    // search for locator roots
    if (!findRoots(iDegree))
      return false;

    // evaluate error values and correct them
    correctErrors(pVector, iDegree);
  }

  if (pMessage) {
    for (i = 0; i < m_iK; i++) {
      pMessage[i] = pVector[m_pMessageSet[i]];
    }
  }

  return true;
}

// --------------------------------------------------------------------------

bool CCyclicCode::decode_soft(value_type *pHardDecisionVector, double *pReliabilitiesVector, value_type* pCodeword /* = NULL */, value_type* pMessage /* = NULL */) {
  int i, iNumOfErasures;
  int iBestDist, iDist;
  bool bDecoded;  
  double dThreshold;

  if (!pCodeword)
    pCodeword = m_pDecodedWordBest;

  bDecoded = decode(pHardDecisionVector, NULL, m_pDecodedWord);

  if (bDecoded) {
    memcpy(pCodeword, m_pDecodedWord, m_iN * sizeof(value_type));  
    iDist = 0;
    for (i = 0; i < m_iN; i++) {
      if (m_pDecodedWord[i] != pHardDecisionVector[i])
        iDist++;
    }
    iBestDist = iDist;
  }
  else {
    memcpy(pCodeword, pHardDecisionVector, m_iN * sizeof(value_type)); 
    iBestDist = m_iN;
  }

  memset(m_pErasuresPositions, 0, m_iN * sizeof(char));
  memcpy(m_pThresholds, pReliabilitiesVector, m_iN * sizeof(double));
  std::sort(m_pThresholds, m_pThresholds + m_iN);

  for (iNumOfErasures = 0; iNumOfErasures < m_iMinDist - 1; ) {
    dThreshold = m_pThresholds[iNumOfErasures + 1]; // uncomment in release
    for (int i = 0; i < m_iN; i++) {
      if (pReliabilitiesVector[i] <= dThreshold && m_pErasuresPositions[i] == 0) {
        m_pErasures[iNumOfErasures++] = i;
        m_pErasuresPositions[i] = 1;
        if ((iNumOfErasures & 1) == 0 || iNumOfErasures == m_iMinDist - 1)
          break;
      }
    }

    if (decode(pHardDecisionVector, NULL, m_pDecodedWord, iNumOfErasures, m_pErasures)) {
      bDecoded = true;
      iDist = 0;
      for (i = 0; i < m_iN; i++) {
        if (m_pDecodedWord[i] != pHardDecisionVector[i])
          iDist++;
      }
      if (iDist < iBestDist) {
        memcpy(pCodeword, m_pDecodedWord, m_iN * sizeof(value_type));  
        iBestDist = iDist;
      }
      else {
        if (iDist > iBestDist)
          break;
      }
    }
  }

  if (pMessage) {
    for (i = 0; i < m_iK; i++) {
      pMessage[i] = pCodeword[m_pMessageSet[i]];
    }
  }

  if (pCodeword == m_pDecodedWordBest)
    memcpy(pHardDecisionVector, pCodeword, m_iN * sizeof(int));

  return bDecoded;
}

// --------------------------------------------------------------------------

void CCyclicCode::allocateDecoderBuffers() {
  // allocate decoder buffers
  m_pDecodedWordBest = new value_type[m_iN];
  if (!m_pDecodedWordBest)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_pThresholds = new double[m_iN];
  if (!m_pThresholds)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
  
  m_pDecodedWord = new value_type[m_iN]; // ?? почему то надо дополнять по ближайшей степени двойки
  if (!m_pDecodedWord)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_pErasures = new int[m_iN];
  if (!m_pErasures)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_pErasuresPositions = new char[m_iN];
  if (!m_pErasuresPositions)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_pSyndromes = new value_type[m_iN];
  if (!m_pSyndromes)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
  
  m_pSigma = new value_type[m_iN];
  if (!m_pSigma)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
  
  m_pB = new value_type[m_iN];
  if (!m_pB)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
  
  m_pT = new value_type[m_iN + 1];
  if (!m_pT)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_pRoots = new value_type[m_iN];
  if (!m_pRoots)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_pReliabilities = new double[m_iN];
  if (!m_pReliabilities)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);
  
  m_pHardDecision = new value_type[m_iN];
  if (!m_pHardDecision)
    throw CCyclicCodeException(CCyclicCodeException::exceptionMemoryAllocationError);

  m_bDecoderInitialized = true;
}

// --------------------------------------------------------------------------

// возврящает true, если все синдромы = 0 (не было ошибок)
bool CCyclicCode::calculateSyndromes(value_type *pVector) {
  int i, j;
  value_type val;
  bool bHasErrors = false;

  m_pSyndromes[0] = 1;
  bHasErrors = false;
  for (i = 0; i < m_iR; i++) {
    val = pVector[m_iN - 1];
    for (j = m_iN - 2; j >= 0; j--) {
      val = m_pGFRoots->sum(pVector[j], m_pGFRoots->mul(val, m_pGFRoots->pow(2, i + m_iOffset)));
    }

    m_pSyndromes[i + 1] = val;
    if (val != 0)
      bHasErrors = true;
  }
  // printvectorf("sd: ", m_pSyndromes, m_iR + 1, "%3d ", "syndromes.txt");

  return !bHasErrors;
}

// --------------------------------------------------------------------------

//pSigma should be initialized by erasure-locator polynomial

int CCyclicCode::BerlekampMassey(int iNumOfErasures) {
  int i, j, L;
  value_type delta;

  // step 2.1
  j = iNumOfErasures;
  L = iNumOfErasures;

  memcpy(m_pB, m_pSigma, m_iMinDist * sizeof(value_type));
  memset(m_pT, 0, (m_iMinDist + 1) * sizeof(value_type));

/*
  {
    printvectorf("j: ", &j, 1, "%3d ", "bm.txt", "w");
    printvectorf("L: ", &L, 1, "%3d ", "bm.txt", "a");
    printvectorf("T: ", m_pT, m_iR + 1, "%3d ", "bm.txt", "a");
    printvectorf("B: ", m_pB, m_iR + 1, "%3d ", "bm.txt", "a");
    printvectorf("S: ", m_pSigma, m_iR + 1, "%3d ", "bm.txt", "a");
  }
*/
  while (++j < ((m_iMinDist % 2 == 0) ? m_iMinDist - 1 : m_iMinDist)) {
    // step 2.3
    // delta = 0;
    delta = m_pSyndromes[j];
    for (i = 1; i <= L; i++) {
      delta = m_pGFRoots->sum(delta, m_pGFRoots->mul(m_pSigma[i], m_pSyndromes[j - i]));
    }

    // step 2.4
    if (delta != 0) {
      // step 2.5
      memmove(m_pB + 1, m_pB, (m_iMinDist - 1) * sizeof(value_type));
      m_pB[0] = 0;

      for (i = 0; i < m_iMinDist; i++) {
        m_pT[i] = (m_pB[i] != 0) ? m_pGFRoots->sub(m_pSigma[i], m_pGFRoots->mul(delta, m_pB[i])) : m_pSigma[i];
      }

      // step 2.6
      if (2 * L <= j - 1/* + iNumOfErasures*/) { // was modified for erasures
        // step 2.7
        for (i = 0; i < m_iMinDist; i++) {
          m_pB[i] = m_pGFRoots->div(m_pSigma[i], delta);
        }

        memcpy(m_pSigma, m_pT, m_iMinDist * sizeof(value_type));

        L = j - L + iNumOfErasures; // was modified for erasures
      }
      else {
        // step 2.8
        memcpy(m_pSigma, m_pT, m_iMinDist * sizeof(value_type));

        // step 2.9 (made at step 2.5, first two lines)
      }
    }
    else {
      // step 2.9
      memmove(m_pB + 1, m_pB, (m_iMinDist - 1) * sizeof(value_type));
      m_pB[0] = 0;
    }

/*
    {
      printvectorf("j: ", &j, 1, "%3d ", "bm.txt", "a");
      printvectorf("L: ", &L, 1, "%3d ", "bm.txt", "a");
      printvectorf("d: ", &delta, 1, "%3d ", "bm.txt", "a");
      printvectorf("T: ", m_pT, m_iR + 1, "%3d ", "bm.txt", "a");
      printvectorf("B: ", m_pB, m_iR + 1, "%3d ", "bm.txt", "a");
      printvectorf("S: ", m_pSigma, m_iR + 1, "%3d ", "bm.txt", "a");
    }
*/
  }

  // step 2.11
  int iDeg = 0;
  for (i = 1; i < m_iMinDist; i++) {
    if (m_pSigma[i])
      iDeg = i;
  }

  // printvectorf("sd: ", m_pSigma, m_iR + 1, "%3d ", "sigma.txt");

  return (iDeg == L) ? L : -1;
}

// --------------------------------------------------------------------------

bool CCyclicCode::findRoots(int iSigmaDegree) {
  int i, j, iRCtr = 0;
  value_type val, pt;

  for (i = 0; i < m_pGFRoots->getCardinality() - 1; i++) {
    pt = m_pGFRoots->pow(2, i + m_iOffset);

    val = m_pSigma[iSigmaDegree];
    for (j = iSigmaDegree - 1; j >= 0; j--) {
      val = m_pGFRoots->sum(m_pSigma[j], m_pGFRoots->mul(val, pt));
    }

/*
    if (val == 0 && 0) {
      printf("%d - %d", i, val);
      if (val == 0)
        printf(" - %d", m_pGFRoots->inv(pt));
      printf("\n");
    }
*/

    if (val == 0)  // (i + m_iOffset) is a root
      m_pRoots[iRCtr++] = pt;
  }

/*
  for (i = 0; i < iRCtr; i++) 
    m_pB[i] = m_pGFRoots->inv(m_pRoots[i]);

  printvectorf("roots   : ", m_pRoots, iRCtr, "%3d ", "roots.txt", "w");
  printvectorf("locators: ", m_pB, iRCtr, "%3d ", "roots.txt", "a");
  
  for (i = 0; i < iRCtr; i++) 
    assert(m_pB[i] - 1 < m_iN);
*/

  return (iRCtr == iSigmaDegree);
}

// --------------------------------------------------------------------------

void CCyclicCode::correctErrors(value_type *pVector, int iNumOfErrorsAndErasures) {
  int i, j;

//   if (m_pGF->getCardinality() == 2) {
//     for (i = 0; i < iNumOfErrorsAndErasures; i++) 
//       pVector[m_pGFRoots->inv(m_pRoots[i]) - 1] ^= 1;
//   }
//   else {
    // evaluate formal derivative b(x) = sigma'(x), degree is iNumOfEaE - 1 or less
    memcpy(m_pB, m_pSigma + 1, (iNumOfErrorsAndErasures) * sizeof(value_type));
    if ((m_pGF->getCardinality() & 1) == 0) {
      for (i = 0; i < iNumOfErrorsAndErasures; i++) {
        if ((i & 1) == 1)
          m_pB[i] = 0;
      }
    } else {
      for (i = 0; i < iNumOfErrorsAndErasures; i++) {
        value_type val = m_pB[i];
        for (j = 0; j < i; j++) 
          m_pB[i] = m_pGFRoots->sum(m_pB[i], val);
      }
    }

    // evaluate errors-and-erasures evaluator polynomial t(x) = sigma(x) * S(x) mod x^Dmin
    // S(x) = 1 + S1 * x + S2 * x^2 + ...
    memset(m_pT, 0, m_iMinDist * sizeof(value_type));
    for (j = 0; j <= m_iR; j++) {
      for (i = 0; i <= iNumOfErrorsAndErasures; i++) {
        if (i + j < m_iMinDist) 
          m_pT[i + j] = m_pGFRoots->sum(m_pT[i + j], m_pGFRoots->mul(m_pSyndromes[j], m_pSigma[i]));
      }
    }

/*
    printvectorf("sg: ", m_pSigma, iNumOfErrorsAndErasures + 1, "%3d ", "sigma.txt", "w");
    printvectorf("fd: ", m_pB, iNumOfErrorsAndErasures, "%3d ", "sigma.txt", "a");
    printvectorf("ee: ", m_pT, m_iMinDist, "%3d ", "sigma.txt", "a");
*/

    // evaluate each error and erasure value
    for (i = 0; i < iNumOfErrorsAndErasures; i++) {
      value_type val, tmp, inv;

      inv = m_pGFRoots->inv(m_pRoots[i]);
      if (inv - 1 >= m_iN)
        continue; // skip locator which is more then m_iN
  	  tmp = m_pGFRoots->pow(inv, 2 - m_iOffset);
      
      val = m_pT[m_iMinDist - 1];
      for (j = m_iMinDist - 2; j >= 0; j--) {
        val = m_pGFRoots->sum(m_pT[j], m_pGFRoots->mul(val, m_pRoots[i]));
      }

      tmp = m_pGFRoots->mul(tmp, val);

      val = m_pB[iNumOfErrorsAndErasures - 1];
      for (j = iNumOfErrorsAndErasures - 2; j >= 0; j--) {
        val = m_pGFRoots->sum(m_pB[j], m_pGFRoots->mul(val, m_pRoots[i]));
      }

      pVector[inv - 1] = m_pGFRoots->sub(pVector[inv - 1], m_pGFRoots->div(tmp, val));
    }

//   }
}

// --------------------------------------------------------------------------

