/** Exception class for cyclic code classes header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 06.11.2003
 */

# ifndef   CYCLICCODEEXCEPTION_H
#   define CYCLICCODEEXCEPTION_H

// [ Import declarations ]

// --------------------------------------------------------------------------
// [ Export declarations: class CCyclicCodeException ]

class CCyclicCodeException {
  // Exception subtypes
  public:
    enum EType {
      exceptionMemoryAllocationError = 1,
      exceptionCodeAlreadyInitialized,
      exceptionMatrixReductionError,
      exceptionCodeIsNotInitialized,
    };

  // Construction / Destruction
  public:
    CCyclicCodeException(int iExceptionCode);

  // Operations
  public:
    void toString(char *strExceptionInfo);
    static void toString(int iExceptionCode, char *strExceptionInfo);
	
    int  getCode();
	
  // Implementation
  protected:
    int m_iExceptionCode;
};


#endif // CYCLICCODEEXCEPTION_H
