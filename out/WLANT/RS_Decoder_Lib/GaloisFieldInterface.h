 /** Galois Field and Extended Galois Field interface file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 04.11.2003
 */

# ifndef   GALOISFIELDINTERFACE_H
#   define GALOISFIELDINTERFACE_H

// --------------------------------------------------------------------------
// [ Export declarations: class IGaloisField ]

class CGaloisPolynomial;

class IGaloisField {
  // Operations
  public:
    // Field Arithmetic operations
    virtual int  sum(int a, int b) = 0;
    virtual int  sub(int a, int b) = 0;
    virtual int  mul(int a, int b) = 0;
    virtual int  div(int a, int b) = 0; 

    virtual int  inv(int a) = 0;

    virtual int  mul_base(int a, int b) = 0; // b - element from base field
    virtual int  pow(int a, int b) = 0; // returns a^b

    // Returns number of elements
    virtual int  getCardinality() = 0;
    virtual int  getExtension() = 0;

    // Returns true, if this interface corresponds to base field (not extended)
    virtual bool isBase() = 0;
    virtual IGaloisField* getBase() = 0;

    virtual int  normalize(int element) = 0;

    virtual CGaloisPolynomial* getElementPoly(int iElemNum) = 0;

    virtual ~IGaloisField() {};  
};

// Helper functions
IGaloisField* createGF2sm(int s, int m);
IGaloisField* createGF2s(int s);

#endif // GALOISFIELDINTERFACE_H
