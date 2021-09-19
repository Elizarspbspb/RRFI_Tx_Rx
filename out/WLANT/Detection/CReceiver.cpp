#include "CReceiver.h"

CReceiver::CReceiver(void)
{
	GSLSample = new CGSLComplex(0,0);
	m_isFileInput = true;
	m_iProcessedSamples = 0;
}

CReceiver::~CReceiver(void)
{
}

void CReceiver::takeControl()
{
	throwPoint();
}

string CReceiver::test()
{
	string tmp("CReceiver::test");
	return tmp;
}

void CReceiver::throwPoint()
{
	double dRe, dIm;
	
	if(m_isFileInput)
	{
		m_InputFile >> dRe;
		m_InputFile >> dIm;
	}
	else
	{
		dRe = m_pSignal[m_iProcessedSamples].dat[0];
		dIm = m_pSignal[m_iProcessedSamples].dat[1];
	}
	m_iProcessedSamples++;

	DISPLAY("Receive  : " << dRe << " " << dIm << endl);

	GSLSample->imag(dIm);
	GSLSample->real(dRe);

	m_pBusOutput->pushEl(*GSLSample);
}

void CReceiver::setInputFile(string filename)
{
	m_InputFile.open(filename.c_str());
	m_isFileInput = true;
}

void CReceiver::setInputSignal(gsl_complex *pSignal)
{
	m_pSignal = pSignal;
	m_isFileInput = false;
}