#pragma once

#include "Includes.h"
#include "CBlock.h"
#include "FXPComplex/FXPComplex16.h"
#include "GSLComplex/CGSLComplex.h"

#include "CMailClient.h"
#include "CMail.h"
#include "CADC.h"

///Class for float-point signal level control
class CVGA : public CBlock<CGSLComplex, CGSLComplex>, public CMailClient
{
public:
	CVGA(void);
	~CVGA(void);

	void takeControl();
	void reset();
	string test();

	void setGainPerSample(double dGainChangePerSample) {m_dGainChangePerSample = dGainChangePerSample; };
	double getGainPerSample() {return m_dGainChangePerSample; };

	void setTagetGain(int iTagetGain) 
	{ 
		m_iTagetGain = iTagetGain>0   ? iTagetGain : 0; 
		m_iTagetGain = iTagetGain<127 ? m_iTagetGain : 127;
		//m_iTagetGain = 59;

		CONSOLECOLOR(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		DISPLAY("VGA : setGain(is : " << m_iCurrentGain << " want : " << m_iTagetGain << ")" << endl);
		CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	};
	int getTagetGain() { return m_iTagetGain; };

	int getCurrentGain() { return m_iCurrentGain; };

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	///Total number of processed samples
	unsigned m_iProcessedSamples;

	///Gain changing velocity
	double m_dGainChangePerSample;

	///Current VGA gain
	int m_iCurrentGain;
	///Requered by AGC gain
	int m_iTagetGain;
};