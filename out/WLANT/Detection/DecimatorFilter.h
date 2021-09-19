#pragma once

#include "Includes.h"
#include "CBlock.h"
#include "CThroughVector.h"
#include "FXPComplex/FXPComplex16.h"

class CDecimatorFilter : public CBlock<CFXPComplex16, CFXPComplex16>
{
public:
	CDecimatorFilter() ;
	CDecimatorFilter(int iDecimationRate);
	~CDecimatorFilter(void){};

	void reset(int iDecimationRate);

	void takeControl();

	string test();

	/////////////////////////////////////////----- private -----//////////////////////////////////////////////

private:
	float m_filter2[30]; 
	int m_iFilterLength;
	int m_iFilterHalfLength;

	vector<CFXPComplex16> m_vBuffer;
	vector<CFXPComplex16> m_vInput;

	int mi1,mi2,mi3,mi4;
	int m_iSkipEl;

private:
	int m_iDecimationRate;
	int m_iStroke;
	unsigned m_uProcessedSamples;
};
