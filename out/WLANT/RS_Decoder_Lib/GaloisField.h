/** Galois Field header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 13.09.2003
 */

# ifndef   GALOISFIELD_H
#   define GALOISFIELD_H

// [ Import declarations ]

#include "GaloisFieldInterface.h"

// --------------------------------------------------------------------------
// [ Export declarations: class CGaloisField ]

class CGaloisPolynomial;

class CGaloisField: public IGaloisField {
	// Construction / Destruction
  public:
    CGaloisField(int p);
    virtual ~CGaloisField();

  // Operations
  public:
    virtual int  sum(int a, int b);
    virtual int  sub(int a, int b);
    virtual int  mul(int a, int b);
    virtual int  div(int a, int b); 

    virtual int  mul_base(int a, int b); // equal to mul(a,b)

    virtual int  inv(int element);
    
    virtual int  pow(int a, int b);

    virtual int  getCardinality();
    virtual int  getExtension();

    virtual bool isBase();
    virtual IGaloisField* getBase();

    virtual CGaloisPolynomial* getElementPoly(int iElemNum);
    
  // Implementation
  protected:
    int normalize(int element);

    // Field cardinality
    int m_iCard; 
};


#endif // GALOISFIELDIMPL_H
