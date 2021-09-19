  /** Base class for any cyclic code header file
 *
 * @author: Andrey Efimov
 * @organization: LCA, ISS Dept@SUAI
 * @date: 06.11.2003
 */

# ifndef   CYCLICCODE_H
#   define CYCLICCODE_H

// [ Import declarations ]

#include <stdio.h>

// --------------------------------------------------------------------------
// [ Export declarations: class CCyclicCode ]

class IGaloisField;

class CCyclicCode {
  public:
    typedef int value_type;

  // Construction / destruction
  public:
    CCyclicCode();
    virtual ~CCyclicCode();
    
  // Initialization
  public:
    
    void initRSCode(IGaloisField *pGF, int iMinDist, int iNShortened, int iOffset = 1);
    void initRSCodePoly(IGaloisField *pGF, int iMinDist, int iNShortened, int iOffset = 1);

    void initBCHfromRSCode(IGaloisField *pGFRS, int iRSMinDist, int iOffset = 1);

    void initFromH(value_type *H, int iR, int iN, IGaloisField *pGF, int iMinDist = -1, IGaloisField *pGFExt = NULL, int iOffset = 1, bool bReduce = false);
    void initFromG(value_type *G, int iK, int iN, IGaloisField *pGF, int iMinDist = -1, IGaloisField *pGFExt = NULL, int iOffset = 1, bool bReduce = false);
    void initFromGFile(const char *sFileName, int iK, int iN, IGaloisField *pGF, int iMinDist = -1, IGaloisField *pGFExt = NULL, int iOffset = 1, bool bReduce = false);
    
    void encode(value_type *pMessage, value_type* pCodeword);
    bool decode(value_type* pVector, value_type* pMessage = NULL, value_type* pCodeword = NULL, int iNumOfErasures = 0, int *pErasureLocations = NULL);
    bool decode_soft(value_type *pHardDecisionVector, double *pReliabilitiesVector, value_type* pCodeword = NULL, value_type* pMessage = NULL);

    void encode(const unsigned char *pMessage, unsigned char *pCodeword);
    void decode_soft(const float *pReliabilities, unsigned char *pMessage);

  // Code properties
  public:
    int  getCodewordLength();
    int  getMessageLength();
    int  getMinDistance();

  // Implementation
  protected:
    bool calculateSyndromes(value_type *pVector);
    int  BerlekampMassey(int iNumOfErasures);
    bool findRoots(int iDegree);
    void correctErrors(value_type *pVector, int iNumOfErrorsAndErasures);
    
    void allocateDecoderBuffers();
    
  protected:
    value_type *m_pG;         // generative matrix of a code
    value_type *m_pH;         // parity check matrix of a code
    value_type *m_pGenPoly;   // generative polynomial of a code
    value_type *m_pMessageSet; 

    int  m_iN;                // code length
    int  m_iK;                // code dimension
    int  m_iR;                // number of parity checks 
    int  m_iMinDist;          // minimum distance of a code
    int  m_iOffset;

    IGaloisField *m_pGF;      // pointer to Galois Field arithmetic
    bool m_bGFAutoDelete;
    IGaloisField *m_pGFRoots; // GF in which all roots of error-locator polynomial are contained in

    value_type *m_pSyndromes; // array of m_iR syndromes
    value_type *m_pSigma;     // coefficients fof a error-locator polynomial
    value_type *m_pB, *m_pT;  // arrays of m_iN length
    value_type *m_pRoots;     // roots of error-locator polynomial

    value_type *m_pDecodedWord;
    value_type *m_pDecodedWordBest;
    double     *m_pThresholds;
    int        *m_pErasures;
    char       *m_pErasuresPositions;

    double     *m_pReliabilities;
    value_type *m_pHardDecision;

    bool m_bInitialized;
    bool m_bDecoderInitialized;
};

#endif // CYCLICCODE_H
