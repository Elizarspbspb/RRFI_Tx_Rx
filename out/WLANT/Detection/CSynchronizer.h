#pragma once

#include "Includes.h"
#include "CAcquisitior.h"
#include "CADC.h"
#include "CAGC.h"
#include "CVGA.h"
#include "CReceiver.h"
#include "CMail.h"
#include "CBus.h"
#include "CSplitter.h"
#include "CDecimator.h"
#include "DecimatorFilter.h"
#include "CModelParameters.h"
#include "GSLComplex/CGSLComplex.h"
#include "FXPComplex/FXPComplex16.h"

//!Class, that manage time between all blocks
class CSynchronizer
{
public:
	CSynchronizer(void);
	~CSynchronizer(void);

	void initSynchronizer(bool isFileInput = true, gsl_complex *pSignal = NULL);
	void initModelParameters(unsigned uPrecision, unsigned uExpPosition, unsigned uAcquisitiorDecimationRate, int iADCMax,
		unsigned uCorrelationLength, unsigned uCorrelationDelay, unsigned uAcquisitiorWindowSize);
	void run(unsigned uSamplesToRead, bool isControlReturn = false);

	int getCurrentGain();
	unsigned getCurrentSample();

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	//Model objects
	CAcquisitior *m_pAcquisitior;
	CADC *m_pADC;
	CAGC *m_pAGC;
	CVGA *m_pVGA;

	//System objects
	CMail *m_pMail;
	CReceiver *m_pReceiver;
	CSplitter<CFXPComplex16> *m_pSplitter;
	CDecimator<CFXPComplex16> *m_pDecimator;
	//CDecimatorFilter *m_pDecimator;
	CModelParameters *m_pModelParameters;

	map<string, CBus<CGSLComplex>*> m_GSLBusMap;
	map<string, CBus<CFXPComplex16>*> m_FXPBusMap;

	unsigned m_iProcessedSamples;

private:
	void initBusSystem();
};
