#pragma once

#include "Includes.h"
#include "CBeginBlock.h"
#include "FXPComplex/FXPComplex16.h"
#include "GSLComplex/CGSLComplex.h"

//!Class, export data to detection cascade
class CReceiver : public CBeginBlock<CGSLComplex>
{
public:
	CReceiver();
	~CReceiver();

	void takeControl();
	string test();

	void setInputFile(string filename);
	void setInputSignal(gsl_complex *pSignal);

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	CGSLComplex *GSLSample;

	ifstream m_InputFile;
	gsl_complex *m_pSignal;
	bool m_isFileInput;

	unsigned m_iProcessedSamples;

private:
	void throwPoint();
};
