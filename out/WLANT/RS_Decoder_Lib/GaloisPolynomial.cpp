                                /** Polynomials under Galois Fields implementation file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 13.09.2003 
 */

# ifndef     GALOISPOLYNOMIAL_H
#   include "GaloisPolynomial.h"
# endif

#include <stdio.h>
#include <memory.h>
#include "GaloisFieldInterface.h"
//#include <iostream.h>
#include <iostream>
using namespace std;

#define max(a,b)    (((a) > (b)) ? (a) : (b))

// --------------------------------------------------------------------------

CGaloisPolynomial::CGaloisPolynomial(IGaloisField *pField, int iDegree /* = 0 */) 
: m_iDegree(iDegree)
, m_pField (pField)
{
  m_vCoefs.resize(m_iDegree + 1);
  for (int ctr = 0; ctr <= m_iDegree; ctr++) 
		m_vCoefs[ctr] = 0;
}

// --------------------------------------------------------------------------

CGaloisPolynomial::CGaloisPolynomial(const int *pCoefs, int iDegree, IGaloisField *pField)
: m_iDegree (iDegree)
, m_pField (pField)
{
	while (m_iDegree > 0 && pField->normalize(pCoefs[m_iDegree]) == 0) m_iDegree--;
  m_vCoefs.resize(m_iDegree + 1);

  for (int ctr = 0; ctr <= m_iDegree; ctr++) 
		m_vCoefs[ctr] = pField->normalize(pCoefs[ctr]);
}

// --------------------------------------------------------------------------

CGaloisPolynomial::CGaloisPolynomial(const std::vector<int> &vCoefs, IGaloisField *pField)
: m_iDegree (vCoefs.size() - 1)
, m_pField (pField)
, m_vCoefs (vCoefs)
{
  trimPower();
}

// --------------------------------------------------------------------------

CGaloisPolynomial::CGaloisPolynomial(const CGaloisPolynomial &poly) {
  *this = poly;
}

// --------------------------------------------------------------------------

CGaloisPolynomial::~CGaloisPolynomial() {
}

// --------------------------------------------------------------------------

int CGaloisPolynomial::getDegree() const {
  return m_iDegree;
}

// --------------------------------------------------------------------------

int CGaloisPolynomial::calculate(int element) const {
	int deg = getDegree();
  int res = getCoefficient(deg);
  for (int j = deg - 1; j >= 0; j--) {
    res = m_pField->sum(getCoefficient(j), m_pField->mul(res, element));
  }
	return res;
}

// --------------------------------------------------------------------------

IGaloisField* CGaloisPolynomial::getField() const {
  return m_pField;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::sum(const CGaloisPolynomial &poly) const {
	int degrees = max(getDegree(), poly.getDegree());
  std::vector<int> sumcoefs(degrees + 1);

	for (int ctr = 0; ctr <= degrees; ctr++) 
		sumcoefs[ctr] = getField()->sum(getCoefficient(ctr), poly.getCoefficient(ctr)); 
  
  return new CGaloisPolynomial(sumcoefs, getField());
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::sub(const CGaloisPolynomial &poly) const {
	int degrees = max(getDegree(), poly.getDegree());
  std::vector<int> sumcoefs(degrees + 1);

	for (int ctr = 0; ctr <= degrees; ctr++) 
		sumcoefs[ctr] = getField()->sub(getCoefficient(ctr), poly.getCoefficient(ctr)); 

  return new CGaloisPolynomial(sumcoefs, getField());
}

// --------------------------------------------------------------------------

CGaloisPolynomial& CGaloisPolynomial::plus(const CGaloisPolynomial &poly) {
	int degrees = max(getDegree(), poly.getDegree());
	for (int ctr = 0; ctr <= degrees; ctr++) 
		setCoefficient(ctr, getField()->sum(getCoefficient(ctr), poly.getCoefficient(ctr))); 
  return *this;
}

// --------------------------------------------------------------------------

CGaloisPolynomial& CGaloisPolynomial::minus(const CGaloisPolynomial &poly) {
	int degrees = max(getDegree(), poly.getDegree());
	for (int ctr = 0; ctr <= degrees; ctr++) 
		setCoefficient(ctr, getField()->sub(getCoefficient(ctr), poly.getCoefficient(ctr))); 
  return *this;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::mul(const CGaloisPolynomial &poly) const {
	int degrees = getDegree() + poly.getDegree();
  std::vector<int> prodcoefs(degrees + 1);

	for (int ctr = 0; ctr <= degrees; ctr++) {
    int coef = 0;
		for (int deg = ctr; deg >= 0; deg--) 
			coef = m_pField->sum(coef, m_pField->mul(getCoefficient(deg), poly.getCoefficient(ctr - deg)));
		prodcoefs[ctr] = coef;
	}

	return new CGaloisPolynomial(prodcoefs, m_pField);
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::mod(const CGaloisPolynomial &poly) const {
	int minpower, nbcoef, nrcoef, mincoef;
	
  CGaloisPolynomial *remainder = new CGaloisPolynomial(*this);
  // printf("  poly: ");
  // ((CGaloisPolynomial*)(&poly))->println();
  nbcoef = poly.getCoefficient(poly.getDegree());
	do {
		minpower = remainder->getDegree() - poly.getDegree();
		if (minpower >= 0) {
      nrcoef = remainder->getCoefficient(remainder->getDegree());
      mincoef = m_pField->div(nrcoef, nbcoef);
			CGaloisPolynomial* minpoly = poly.mul_one_term(mincoef, minpower);
      // printf(  "  minpoly: ");
      // minpoly->println();
      remainder->minus(*minpoly);
      // printf(  "  remainder: ");
      // remainder->println();
      remainder->trimPower();
      delete minpoly;
		}
	} while (minpower > 0);
	return remainder;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::div(const CGaloisPolynomial &poly) const {
  int divDegree = max(0,getDegree() - poly.getDegree()) + 1;
  std::vector<int> divcoefs(divDegree + 1);
	int minpower, nbcoef, nrcoef, mincoef;
	
  CGaloisPolynomial *remainder = new CGaloisPolynomial(*this);
  nbcoef = poly.getCoefficient(poly.getDegree());
	do {
		minpower = remainder->getDegree() - poly.getDegree();
		if (minpower >= 0) {
      nrcoef = remainder->getCoefficient(remainder->getDegree());
      mincoef = m_pField->div(nrcoef, nbcoef);
			CGaloisPolynomial* minpoly = poly.mul_one_term(mincoef, minpower);
      remainder->minus(*minpoly);
      remainder->trimPower();
			divcoefs[minpower] = mincoef;
      delete minpoly;
		}
	} while (minpower > 0);
	return new CGaloisPolynomial(divcoefs, m_pField);
} 

// --------------------------------------------------------------------------
bool CGaloisPolynomial::equals(const CGaloisPolynomial &poly) const {
	int degres = max(getDegree(), poly.getDegree());
	for (int ctr = 0; ctr <= degres; ctr++) 
		if (getCoefficient(ctr) != poly.getCoefficient(ctr)) 
      return false;
	return true;		
}

// --------------------------------------------------------------------------

int CGaloisPolynomial::getCoefficient(int num) const {
  if (num > getDegree()) 
    return 0;
  return m_vCoefs[num];
}

// --------------------------------------------------------------------------

void CGaloisPolynomial::setCoefficient(int num, int val) {
  if (num >= m_vCoefs.size())
    m_vCoefs.resize(num, val);
  else
    m_vCoefs[num] = val;
}

// --------------------------------------------------------------------------

CGaloisPolynomial& CGaloisPolynomial::operator = (const CGaloisPolynomial &poly) {
  m_iDegree = poly.getDegree();
  m_pField = poly.getField();
  m_vCoefs = poly.m_vCoefs;
  return *this;
}

// --------------------------------------------------------------------------

bool CGaloisPolynomial::operator == (const CGaloisPolynomial &poly) const {
  return equals(poly);
}

// --------------------------------------------------------------------------

bool CGaloisPolynomial::operator != (const CGaloisPolynomial &poly) const {
  return !equals(poly);
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::mul_one_term(int coef, int pwr) const {
	int degree = getDegree() + pwr;
  int ctr;
  std::vector<int>prodcoefs(degree + 1);
  
  for (ctr = 0; ctr < pwr; ctr++) 
    prodcoefs[ctr] = 0;
  
  for (ctr = 0; ctr <= getDegree(); ctr++)
    prodcoefs[ctr + pwr] = m_pField->mul(getCoefficient(ctr), coef);		
  
  return new CGaloisPolynomial(prodcoefs, m_pField);
}

// --------------------------------------------------------------------------

void CGaloisPolynomial::print() {
  int len = getDegree();
  bool add = false;
	for (int ctr = 0; ctr <= getDegree(); ctr++) {
		// if (getCoefficient(ctr) != 0 || len == 1) 
    {
			if (add) 
        cout << " + ";
      if (getField()->isBase())
        cout << getCoefficient(ctr);
      else {
        cout << "( <skipped>";
        // ((CGaloisExtendedField*)getField())->getPoly(getCoefficient(ctr) - 1)->print();
        cout << ")";
      }
			if (ctr == 1) 
        cout << "x";
			if (ctr > 1) 
        cout << "x^" << ctr;
			add = true;
		}
	}
}

// --------------------------------------------------------------------------

void CGaloisPolynomial::println() {
  print();
  cout << endl;
}

// --------------------------------------------------------------------------

void CGaloisPolynomial::printCoefs() {
  int len = getDegree();
	for (int ctr = 0; ctr <= getDegree(); ctr++) 
    cout << getCoefficient(ctr);
  cout << endl;
}

// --------------------------------------------------------------------------

void CGaloisPolynomial::trimPower() {
	while (m_iDegree > 0 && m_vCoefs[m_iDegree] == 0) m_iDegree--;
}

// --------------------------------------------------------------------------

CGaloisPolynomial* CGaloisPolynomial::createConstant(int elem, IGaloisField *pField) {
  return new CGaloisPolynomial(&elem, 0, pField);
}

// --------------------------------------------------------------------------
