      /** Extended Galois Field implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 13.09.2003
 */

#include "GaloisExtendedField.h"
#include "GaloisException.h"
#include "GaloisPolynomial.h"

#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include <iostream>

// --------------------------------------------------------------------------

CGaloisExtendedField::CGaloisExtendedField(const int *irrPolyCoefs, int irrPolyDegree, IGaloisField* pBaseField, bool bAutoDeleteBaseField /* = true */)
: m_pBaseField (pBaseField)
, m_pModuloPoly (NULL)
, m_pFieldElementPolys (NULL)
, m_iCard((int)pow(pBaseField->getCardinality(), irrPolyDegree))
, m_pPrimitivePoly (NULL)
, m_bAutoDeleteBaseField(bAutoDeleteBaseField)
, m_iExt (irrPolyDegree)
{
  m_pModuloPoly = new CGaloisPolynomial(irrPolyCoefs, irrPolyDegree, pBaseField);
  createFieldElementPolys();
}

// --------------------------------------------------------------------------

CGaloisExtendedField::CGaloisExtendedField(const CGaloisPolynomial *pPoly, bool bAutoDeleteBaseField /* = true */)
: m_pBaseField (pPoly->getField())
, m_pModuloPoly (new CGaloisPolynomial(*pPoly))
, m_pFieldElementPolys (NULL)
, m_iCard((int)pow(pPoly->getField()->getCardinality(), pPoly->getDegree()))
, m_pPrimitivePoly (NULL)
, m_bAutoDeleteBaseField(bAutoDeleteBaseField)
, m_iExt (pPoly->getDegree())
{
  createFieldElementPolys();
}

// --------------------------------------------------------------------------

CGaloisExtendedField::~CGaloisExtendedField() {
  if (m_pModuloPoly) {
    delete m_pModuloPoly;
    m_pModuloPoly = NULL;
  }

  if (m_pPrimitivePoly) {
    delete m_pPrimitivePoly;
    m_pPrimitivePoly = NULL;
  }

  if (m_pFieldElementPolys) {
    m_pFieldElementPolys--; // because indices start from -1
    for (int ctr = 0; ctr < getCardinality(); ctr++) 
      delete m_pFieldElementPolys[ctr];
    delete [] m_pFieldElementPolys; 
    m_pFieldElementPolys = NULL;
  }

  if (m_bAutoDeleteBaseField) { // delete base field
    if (m_pBaseField) {
      delete m_pBaseField;
      m_pBaseField = NULL;
    }
  }
}

// --------------------------------------------------------------------------

void CGaloisExtendedField::createFieldElementPolys() {
  int degree = m_pModuloPoly->getDegree();

  // fill in primitive polynomial
  std::vector<int> coefs(degree + 1);
  coefs[1] = 1;
  m_pPrimitivePoly = new CGaloisPolynomial(coefs, m_pBaseField);
  // m_pPrimitivePoly->print();

  // fill in element's polynomials
  m_pFieldElementPolys = new CGaloisPolynomial*[getCardinality()];
  
  // zero element
  coefs[1] = 0;
  m_pFieldElementPolys[0] = new CGaloisPolynomial(coefs, m_pBaseField);
  
  // others 
  coefs[0] = 1;
  CGaloisPolynomial *betw = new CGaloisPolynomial(coefs, m_pBaseField);

  CGaloisPolynomial *one = new CGaloisPolynomial(*betw);

  for (int ctr = 1; ctr < getCardinality(); ctr++) {
    m_pFieldElementPolys[ctr] = new CGaloisPolynomial(*betw);

    CGaloisPolynomial *polyProd = betw->mul(*m_pPrimitivePoly);
    // polyProd->print();
    delete betw;
    betw = polyProd->mod(*m_pModuloPoly);
    // betw->print();      
    delete polyProd;

    assert(*betw != *one || ctr == getCardinality() - 1); 
  }
  assert(*betw == *one); 

  delete one;
  delete betw;

  m_pFieldElementPolys++; // because indices start from -1
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::sum(int a, int b) {
  int c, d, m_iCard = getCardinality();
  CGaloisPolynomial *ap, *bp, *res;
  c = normalize(a);
  d = normalize(b);
  if (c == 0)
    return d;
  if (d == 0) 
    return c;
  
  ap = m_pFieldElementPolys[c - 1];
  bp = m_pFieldElementPolys[d - 1];
  res = ap->sum(*bp);

  // printf("ap: ");
  // ap->println();
  // printf("bp: ");
  // bp->println();
  // printf("res: ");
  // res->println();
  // printf("\n");

  for (int ctr = -1; ctr < m_iCard - 1; ctr++) {
    if (*res == *m_pFieldElementPolys[ctr]) {
      delete res;
      return ctr + 1;
    }
  }
  delete res;
  return 0;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::sub(int a, int b) {
  int c, d, m_iCard = getCardinality();
  CGaloisPolynomial *ap, *bp, *res;
  c = normalize(a);
  d = normalize(b);
  if (d == 0) 
    return c;

  ap = m_pFieldElementPolys[c - 1];
  bp = m_pFieldElementPolys[d - 1];
  res = ap->sub(*bp);

  for (int ctr = -1; ctr < m_iCard - 1; ctr++) {
    if (*res == *m_pFieldElementPolys[ctr]) {
      delete res;
      return ctr + 1;
    }
  }
  delete res;
  return 0;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::mul(int a, int b) {
	int c, d, e;
	c = normalize(a);
	d = normalize(b);
  if (c == 0 || d == 0) 
    return 0;		
	e = (c + d - 2) % (m_iCard - 1);
	return e+1;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::div(int a, int b) {
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

int CGaloisExtendedField::inv(int a) {
  int c, e;
	c = normalize(a);
  if (c == 0)
    throw CGaloisException(CGaloisException::exceptionDivisionByZero);
  e = (1 - c);
  return (e >= 0) ? e + 1: e + m_iCard;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::mul_base(int a, int b) { // b - element from base field
  int c, d, m_iCard = getCardinality(), pw;
  CGaloisPolynomial *bp = NULL;
  c = normalize(a);
  d = m_pBaseField->normalize(b);
  
  if (d == 0 || c == 0) 
    return 0;

  bp = new CGaloisPolynomial(&d, 0, m_pBaseField);

  pw = 0;
  for (int ctr = -1; ctr < m_iCard - 1; ctr++) {
    if (*bp == *m_pFieldElementPolys[ctr]) {
      pw = ctr + 1;
      break;
    }
  }
  delete bp;

  return mul(a, pw);
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::pow(int a, int b) {
  int c;
	c = normalize(a);
  if (b == 0)
    return 1;
  if (c == 0)
    return 0;
  return ((c - 1) * b) % (m_iCard - 1) + 1;
}

// --------------------------------------------------------------------------

void CGaloisExtendedField::printElements() {
  for (int ctr = -1; ctr < getCardinality() - 1; ctr ++) 
    m_pFieldElementPolys[ctr]->println();
}

// --------------------------------------------------------------------------

void CGaloisExtendedField::printElementCoeffs() {
  for (int ctr = -1; ctr < getCardinality() - 1; ctr ++) 
    m_pFieldElementPolys[ctr]->printCoefs();
}

// --------------------------------------------------------------------------

bool CGaloisExtendedField::isBase() {
  return false;
}

// --------------------------------------------------------------------------

IGaloisField* CGaloisExtendedField::getBase() {
  return m_pBaseField;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::getCardinality() {
  return m_iCard;
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::normalize(int element) {
	int c = element % m_iCard;
	if (c < 0) c += m_iCard;
	return c;		
}

// --------------------------------------------------------------------------

int CGaloisExtendedField::getExtension() {
  return m_iExt;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisExtendedField::getElementPoly(int iElemNum) {
  return m_pFieldElementPolys[iElemNum - 1];
}

// --------------------------------------------------------------------------

