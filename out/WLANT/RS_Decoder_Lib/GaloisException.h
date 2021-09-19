/** Galois Classes Exception header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 02.10.2003
 */

# ifndef   GALOISEXCEPTION_H
#   define GALOISEXCEPTION_H

// [ Import declarations ]

// --------------------------------------------------------------------------
// [ Export declarations: class CGaloisException ]

class CGaloisException {
  // Exception subtype
  public:
    enum EType {
      exceptionDivisionByZero = 1,
      exceptionGenerativePolynomialNotPrimitive,
      exceptionMemoryAllocationError,
    };

  // Construction / Destruction
  public:
    CGaloisException(int iGaloisExceptionCode);

  // Operations
  public:
    void toString(char *strGaloisExceptionInfo);
    static void toString(int iGaloisExceptionCode, char *strGaloisExceptionInfo);
	
    int  getCode();
	
  // Implementation
  protected:
    int m_iGaloisExceptionCode;
};

#endif // GALOISEXCEPTION_H
