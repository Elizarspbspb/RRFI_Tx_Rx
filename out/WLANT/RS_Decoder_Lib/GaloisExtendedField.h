/** Extended Galois Field header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 13.09.2003
 */

# ifndef   GALOISEXTENDEDFIELD_H
#   define GALOISEXTENDEDFIELD_H

// [ Import declarations ]

// #include "GaloisField.h"
#include "GaloisFieldInterface.h"

// --------------------------------------------------------------------------
// [ Export declarations: class CGaloisExtendedField ]

class CGaloisPolynomial;
// class CGaloisField;

class CGaloisExtendedField: public IGaloisField {
	// Construction / Destruction
  public:
    CGaloisExtendedField(const int *irrPolyCoefs, int irrPolyDegree, IGaloisField* pBaseField, bool bAutoDeleteBaseFields = true);
    CGaloisExtendedField(const CGaloisPolynomial *pPoly, bool bAutoDeleteBaseFields = true);
    virtual ~CGaloisExtendedField();

  // Operations
  public:
    // operations from IGaloisField
    virtual int  sum(int a, int b);
    virtual int  sub(int a, int b);
    virtual int  mul(int a, int b);
    virtual int  div(int a, int b); 

    virtual int  inv(int element);

    virtual int  mul_base(int a, int b); // b - element from base field
    virtual int  pow(int a, int b);

    virtual bool isBase();

    virtual IGaloisField* getBase();

    virtual int  getCardinality();
    virtual int  getExtension();

    virtual CGaloisPolynomial* getElementPoly(int iElemNum);

    // local defined operations
    void printElements();
    void printElementCoeffs();

    // Implementation
  protected:
    int normalize(int element);
    void createFieldElementPolys();

    IGaloisField      *m_pBaseField;
    CGaloisPolynomial *m_pModuloPoly;
    CGaloisPolynomial**m_pFieldElementPolys; // array (CGaloisPoly*), indices [-1..card - 1]
    CGaloisPolynomial *m_pPrimitivePoly;
    bool               m_bAutoDeleteBaseField;

    int m_iCard;
    int m_iExt;
};

#endif // GALOISEXTENDEDFIELD_H
