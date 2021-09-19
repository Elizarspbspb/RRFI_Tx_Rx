/** Extended Galois Field GF(2^m) header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 04.11.2003
 */

# ifndef   GALOISEXTENDEDFIELD2MFAST_H
#   define GALOISEXTENDEDFIELD2MFAST_H

// [ Import declarations ]

#include "GaloisFieldInterface.h"
#include "GaloisField.h"

// --------------------------------------------------------------------------
// [ Export declarations: class CGaloisExtendedField2mFast ]

class CGaloisPolynomial;

class CGaloisExtendedField2mFast: public IGaloisField {
	// Construction / Destruction
  public:
    CGaloisExtendedField2mFast(const int *irrPolyCoefs, int irrPolyDegree);
    CGaloisExtendedField2mFast(const CGaloisPolynomial *pPoly);
    virtual ~CGaloisExtendedField2mFast();

  // Operations
  public:
    // Field Arithmetic operations
    virtual int  sum(int a, int b);
    virtual int  sub(int a, int b);
    virtual int  mul(int a, int b);
    virtual int  div(int a, int b); 

    virtual int  inv(int element);

    virtual int  mul_base(int a, int b); // b - element from base field
    virtual int  pow(int a, int b);

    // Returns number of elements
    virtual int  getCardinality();
    virtual int  getExtension();

    // Returns true, if this interface corresponds to base field (not extended)
    virtual bool isBase();
    virtual IGaloisField* getBase();

    virtual CGaloisPolynomial *getElementPoly(int iElemNum);

    virtual int  normalize(int element);

  // Implementation
  protected:
    void createFieldElementPolys(unsigned int gp);

    int *m_pLogs;
    int *m_pAntiLogs;
    int  m_iCard;
    int  m_iExt;

    static CGaloisField m_sGF2;
    CGaloisPolynomial *m_pElemPoly;
};


#endif // GALOISEXTENDEDFIELD2MFAST_H
