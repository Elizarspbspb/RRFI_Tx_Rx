#pragma once

#include "Includes.h"
#include "CBlock.h"
#include "FXPComplex/FXPComplex16.h"
#include "FXPComplex/FXPComplex32.h"
#include "CVGA.h"
#include "CAcquisitior.h"
#include "CAGCHistory.h"

#include "CMailClient.h"
#include "CMail.h"

class CAGC : public CEndBlock<CFXPComplex16>, public CMailClient
{
public:
	CAGC(void);
	~CAGC(void);

	void takeControl();
	string test();
	void init();

	void setPrecision(unsigned uPrecision);
	void setExpPosition(unsigned uExpPosition);
	void setConstPrecision(unsigned uConstPrecision);

	bool isLockedInLastSamples(unsigned uNumOfSamples);

	void fixPacketStartSample();

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	int round(double a);
	template <class T> vector<T> getVectorElements(vector<T> *vData, unsigned from, unsigned to);
	vector<CFXPComplex16> readInputVector(unsigned uNumOfSamples);
	vector<CFXPComplex16> scanInputVector(unsigned uNumOfSamples);
	void changeInputBus();

private:
	enum AGCStateTypes {UnLock, Lock, Freeze, GainUpdate};
	AGCStateTypes m_AGCState;
	bool m_isAGCUnlock;

	unsigned m_uPrecision;
	unsigned m_uExpPosition;

	unsigned m_uConstPrecision;

	///Total number of processed samples
	unsigned m_iProcessedSamples;
	unsigned m_uFreezeSample;
	unsigned m_uPacketStartSample;

	vector<int> m_vAGCAddrModLUT;

	CAGCHistory m_History;

private:
	unsigned uWaitNextTime;
	int m_iClippingsStroke;

private:
	void initAGCAddrModLUT();

	CFXPComplex32 calculateRMS(vector<CFXPComplex16> vData, unsigned uSamplesNumber = 0);
	void changeGainViaTable(int RMS);
	unsigned checkClippings(vector<CFXPComplex16> vData, unsigned uSamplesNumber = 0);

	//Change gain functions
	void assignGain(int iNewGain);
	void addToGain(int iGainVariation);

	void skipInputSamples(unsigned uNumOfSamples);
};
