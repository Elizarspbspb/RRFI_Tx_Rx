#pragma once

#include "Includes.h"
#include "CBlock.h"
#include "FXPComplex/FXPComplex16.h"
#include "CThroughVector.h"
#include "CAGC.h"
#include "CCorrelator.h"

#include "CMailClient.h"
#include "CMail.h"


class CADC;

//! Class for packet detection
class CAcquisitior : public CEndBlock<CFXPComplex16>, public CMailClient
{
public:
	CAcquisitior(void);
	~CAcquisitior(void);

	void init(unsigned uDelay, unsigned uAutoCorrelationLength);

	void takeControl();
	string test();
	void setDESLog(string filename);

	void setAutoCorrelationDelay(unsigned uDelay);
	void setAutoCorrelationLength(unsigned uAutoCorrelationLength);
	void setWindowSize(unsigned uWindowSize);

	void setPrecision(unsigned uPrecision);
	void setExpPosition(unsigned uExpPosition);

	bool isPacketFound();


/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	unsigned m_uDelay;
	unsigned m_uAutoCorrelationLength;
	unsigned m_uWindowSize;

	unsigned m_uResponseShift;
	unsigned m_uEnergyShift;

	unsigned m_uPrecision;
	unsigned m_uExpPosition;

	///Total number of processed samples
	unsigned m_iProcessedSamples;

	int iTh_exceedings;
	int iTh_exceedings2;
	int iTh_exceedingsW;

	int m_iSleep;

	// Build-in correlator
	CThroughVector<CFXPComplex16> m_vDirectLine;
	CThroughVector<CFXPComplex16> m_vDelayedLine;
	CThroughVector<CFXPComplex16> m_vStorageLine;

	// External correlator
	CCorrelator m_ResponceCorrelator;
	CCorrelator m_EnergyCorrelator;

	// Sliding window
	CThroughVector<CFXPComplex32> m_vWindowResponse;
	CThroughVector<CFXPComplex32> m_vWindowEnergy;

	ofstream m_DESLog;
	bool m_isLogging;

	bool m_isPacketFound;

private:
	CFXPComplex32 calculateAutoCorrelation(vector<CFXPComplex16> vSampling, unsigned uDelay, unsigned uLength);
	CFXPComplex32 calculateAutoCorrelation(vector<CFXPComplex16> vSampling1, vector<CFXPComplex16> vSampling2);

	CFXPComplex32 calculateEnergy(vector<CFXPComplex16> vSampling, unsigned uBegin, unsigned uEnd);
	CFXPComplex32 calculateEnergy(vector<CFXPComplex16> vSampling);
};