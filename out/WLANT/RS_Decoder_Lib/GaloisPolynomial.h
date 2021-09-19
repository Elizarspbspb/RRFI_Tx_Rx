 /** Polynomials under Galois Fields header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 13.09.2003 
 */

# ifndef   GALOISPOLYNOMIAL_H
#   define GALOISPOLYNOMIAL_H

// [ Import declarations ]

#include <vector>

// --------------------------------------------------------------------------
// [ Export declarations: class CGaloisPolynomial ]

class IGaloisField;

class CGaloisPolynomial{
  // Construction / Destruction
  public:
    CGaloisPolynomial(IGaloisField *pField, int iDegree = 0);
    CGaloisPolynomial(const int *pCoefs, int iDegree, IGaloisField *pField);
    CGaloisPolynomial(const std::vector<int>& vCoefs, IGaloisField *pField);
    CGaloisPolynomial(const CGaloisPolynomial &poly);
   ~CGaloisPolynomial();

  // Operations
  public:
    int calculate(int element) const;

    // create new polynomial for return
    CGaloisPolynomial* sum(const CGaloisPolynomial &poly) const;
    CGaloisPolynomial* sub(const CGaloisPolynomial &poly) const;
    CGaloisPolynomial* mul(const CGaloisPolynomial &poly) const;
    CGaloisPolynomial* mod(const CGaloisPolynomial &poly) const;
    CGaloisPolynomial* div(const CGaloisPolynomial &poly) const;

    // modify 'this' polynomial (faster then sum() and sub())
    CGaloisPolynomial& plus(const CGaloisPolynomial &poly);
    CGaloisPolynomial& minus(const CGaloisPolynomial &poly);

    int getDegree() const;

    IGaloisField* getField() const;

    int  getCoefficient(int num) const;
    void setCoefficient(int num, int val);

    bool operator == (const CGaloisPolynomial &poly) const;
    bool operator != (const CGaloisPolynomial &poly) const;

    CGaloisPolynomial& operator = (const CGaloisPolynomial &poly);

    void print();
    void println();
    void printCoefs();

    static CGaloisPolynomial* createConstant(int elem, IGaloisField *pField);
    
  // Implementation
  protected:
    bool equals(const CGaloisPolynomial &poly) const;

    CGaloisPolynomial* mul_one_term(int coef, int pwr) const;
    void trimPower();

    std::vector<int> m_vCoefs;
    int              m_iDegree;
    IGaloisField     *m_pField;
};


#endif // GALOISPOLYNOMIAL_H
