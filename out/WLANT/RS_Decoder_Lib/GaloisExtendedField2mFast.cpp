/** Extended Galois Field GF(2^m) implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 04.11.2003
 */

#include "GaloisExtendedField2mFast.h"
#include "GaloisPolynomial.h"
#include "GaloisException.h"
#include "GaloisField.h"

#include <stdio.h>

// --------------------------------------------------------------------------

CGaloisField CGaloisExtendedField2mFast::m_sGF2(2);

// --------------------------------------------------------------------------

CGaloisExtendedField2mFast::CGaloisExtendedField2mFast(const int *irrPolyCoefs, int irrPolyDegree) 
: m_iCard (1 << irrPolyDegree)
, m_pLogs (NULL)
, m_pAntiLogs (NULL)
, m_iExt (irrPolyDegree)
{
  unsigned int gp = 0;
  for (int i = 0; i <= irrPolyDegree; i++)
    gp |= (irrPolyCoefs[i] & 1) << i;
  createFieldElementPolys(gp);
  
  m_pElemPoly = new CGaloisPolynomial(&m_sGF2, m_iExt - 1); 
  if (!m_pElemPoly)
    throw CGaloisException(CGaloisException::exceptionMemoryAllocationError);
}

// --------------------------------------------------------------------------

CGaloisExtendedField2mFast::CGaloisExtendedField2mFast(const CGaloisPolynomial *pPoly) 
: m_iCard (1 << pPoly->getDegree())
, m_pLogs (NULL)
, m_pAntiLogs (NULL)
, m_iExt (pPoly->getDegree())
{
  unsigned int gp = 0;
  for (int i = 0; i <= pPoly->getDegree(); i++)
    gp |= (pPoly->getCoefficient(i) & 1) << i;
  createFieldElementPolys(gp);

  m_pElemPoly = new CGaloisPolynomial(&m_sGF2, m_iExt - 1);
  if (!m_pElemPoly)
    throw CGaloisException(CGaloisException::exceptionMemoryAllocationError);
}

// --------------------------------------------------------------------------

CGaloisExtendedField2mFast::~CGaloisExtendedField2mFast() {
  if (m_pLogs)
    delete [] m_pLogs;
  if (m_pAntiLogs)
    delete [] m_pAntiLogs;
  if (m_pElemPoly)
    delete m_pElemPoly;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::sum(int a, int b) {
  int c, d;
  c = normalize(a);
  d = normalize(b);
  if (c == 0)
    return d;
  if (d == 0)
    return c;
  return m_pLogs[m_pAntiLogs[c - 1] ^ m_pAntiLogs[d - 1]] + 1;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::sub(int a, int b) {
  return sum(a, b);
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::mul(int a, int b) {
	int c, d, e;
	c = normalize(a);
	d = normalize(b);
  if (c == 0 || d == 0) 
    return 0;		
	e = (c + d - 2) % (m_iCard - 1);
	return e + 1;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::div(int a, int b) {
	int c, d, e;
	c = normalize(a);
	d = normalize(b);
	if (c == 0) 
    return 0;		
	if (d == 0) 
    throw CGaloisException(CGaloisException::exceptionDivisionByZero);
	e = (c - d) % (m_iCard - 1);
  return (e >= 0) ? e + 1: e + m_iCard;
}

// --------------------------------------------------------------------------
 

int CGaloisExtendedField2mFast::inv(int element) {
  int c, e;
	c = normalize(element);
  if (c == 0)
    throw CGaloisException(CGaloisException::exceptionDivisionByZero);
  e = (1 - c);
  return (e >= 0) ? e + 1: e + m_iCard;
}

// --------------------------------------------------------------------------


int CGaloisExtendedField2mFast::mul_base(int a, int b) {
  return (b != 0) ? a : 0;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::pow(int a, int b) {
  int c;
	c = normalize(a);
  if (b == 0)
    return 1;
  if (c == 0)
    return 0;
  return ((c - 1) * b) % (m_iCard - 1) + 1;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::getCardinality() {
  return m_iCard;
}

// --------------------------------------------------------------------------

bool CGaloisExtendedField2mFast::isBase() {
  return false;
}

// --------------------------------------------------------------------------

IGaloisField* CGaloisExtendedField2mFast::getBase() {
  return &m_sGF2;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::normalize(int element) {
	int c = element % m_iCard;
	if (c < 0) c += m_iCard;
	return c;		
}

// --------------------------------------------------------------------------

void CGaloisExtendedField2mFast::createFieldElementPolys(unsigned int gp) {
  m_pLogs = new int[m_iCard];
  m_pAntiLogs = new int[m_iCard];

  if (!m_pLogs || !m_pAntiLogs)
    throw CGaloisException(CGaloisException::exceptionMemoryAllocationError);

  m_pLogs[0] = -1;

  unsigned int v = 1;
  unsigned int i = 0;
  do {
    m_pLogs[v] = i;
    m_pAntiLogs[i] = v;
    ++i;
    v <<= 1;
    if (v >= m_iCard)
       v ^= gp;
  } while (v != 1);
  m_pAntiLogs[i] = v;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField2mFast::getExtension() {
  return m_iExt;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisExtendedField2mFast::getElementPoly(int iElemNum) {
  // construct polynom
  for (int i = 0; i < m_iExt; i++)
    m_pElemPoly->setCoefficient(i, (m_pAntiLogs[iElemNum - 1] >> i) & 1);
  return m_pElemPoly;
}

// --------------------------------------------------------------------------
