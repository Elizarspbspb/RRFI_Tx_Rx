/** Galois Field implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 13.09.2003
 */

#include "GaloisField.h"
#include "GaloisPolynomial.h"
#include "GaloisException.h"

#include <assert.h>

// --------------------------------------------------------------------------

CGaloisField::CGaloisField(int p) 
: m_iCard (p) 
{}

// --------------------------------------------------------------------------

CGaloisField::~CGaloisField() {
}

// --------------------------------------------------------------------------

int CGaloisField::getCardinality() {
  return m_iCard;
}

// --------------------------------------------------------------------------

int CGaloisField::sum(int a, int b) {
  return (a + b) % m_iCard;
}

// --------------------------------------------------------------------------

int CGaloisField::sub(int a, int b) {
  return normalize(a - b);
}

// --------------------------------------------------------------------------

int CGaloisField::mul(int a, int b) {
  return (a * b) % m_iCard;
}

// --------------------------------------------------------------------------

int CGaloisField::div(int a, int b) {
  if (b == 0) 
    throw CGaloisException(CGaloisException::exceptionDivisionByZero);
  return (a * inv(b)) % m_iCard;
}

// --------------------------------------------------------------------------

int CGaloisField::normalize(int element) {
	int c = element % m_iCard;
	if (c < 0) c += m_iCard;
	return c;		
}

// --------------------------------------------------------------------------

int CGaloisField::inv(int element) {
  if (element == 0) 
    throw CGaloisException(CGaloisException::exceptionDivisionByZero);

  // a^-1 = a^(m_iCard - 2) 
  // @todo: change it to Euclid GCD algorithm
	int res = 1;
  for (int ctr = 1; ctr <= m_iCard - 2; ctr++) 
		res = (res * element) % m_iCard;
	
  return res;
}

// --------------------------------------------------------------------------

bool CGaloisField::isBase() {
  return true;
}

// --------------------------------------------------------------------------

IGaloisField* CGaloisField::getBase() {
  return this;
}

// --------------------------------------------------------------------------

int CGaloisField::mul_base(int a, int b) {
  return mul(a, b);
}

// --------------------------------------------------------------------------

int CGaloisField::getExtension() {
  return 1;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisField::getElementPoly(int iElemNum) {
  return NULL;
}

// --------------------------------------------------------------------------

int CGaloisField::pow(int a, int b) {
  int c, e;
	c = normalize(a);
  if (b == 0)
    return 1;
  if (c == 0)
    return 0;
  e = a;
  for (int i = 0; i < b - 1; i++) 
    e = mul(e, b);
  return e;
}

// --------------------------------------------------------------------------


