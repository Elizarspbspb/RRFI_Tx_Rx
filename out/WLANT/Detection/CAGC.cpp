#include "CAGC.h"

CAGC::CAGC(void)
{
	//TRACE(_T("CAGC::CAGC\n"));
	m_isAGCUnlock = true;
	m_iProcessedSamples = 0;

	uWaitNextTime = 0;
}

CAGC::~CAGC(void)
{
	//TRACE(_T("CAGC::~CAGC\n"));
}


void CAGC::init()
{
	m_isAGCUnlock = true;
	m_iProcessedSamples = 0;
	m_iClippingsStroke = 0;
	m_AGCState = UnLock;

	initAGCAddrModLUT();
}

void CAGC::takeControl()
{
	unsigned uNumOfClippings;
	int iRMS;
	CFXPComplex16 RMSMin, RMSMax;
	int iFragmentsLooked;

	bool pithyPass = true, onlyClippingsCheck = false;

	CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY);

	//changeInputBus();

	while(!m_pBusInput->isEmpty() && m_AGCState != Freeze && pithyPass && !onlyClippingsCheck)
	{
		pithyPass = false;
		onlyClippingsCheck = true;

		DISPLAY("AGC  : size : " << m_pBusInput->getSize() << endl);

		if(m_pBusInput->getSize() >= uWaitNextTime && uWaitNextTime!=0)
		{
			skipInputSamples(uWaitNextTime);
			uWaitNextTime = 0;
		}

		if(m_pBusInput->getSize() >= 4 && m_iClippingsStroke == 0)
		{
			pithyPass = true;

			uNumOfClippings = checkClippings(scanInputVector(4));

			DISPLAY("AGC  : clipings : " << uNumOfClippings << endl);

			if(uNumOfClippings)
			{
				// [ Change gain
				switch( uNumOfClippings )
				{
				case 0: break;
				case 1: addToGain(-10); break;
				case 2: addToGain(-10); break;
				case 3: addToGain(-13); break;
				case 4: addToGain(-13); break;
				case 5: addToGain(-16); break;
				case 6: addToGain(-16); break;
				case 7: addToGain(-19); break;
				case 8: addToGain(-19); break;

				default: addToGain(-19); break;
				}
				//]

				skipInputSamples(4);
				uWaitNextTime = 4;

				DISPLAYX("Clippings! Sample : " << m_iProcessedSamples << endl);

				m_History.addEvent(m_iProcessedSamples-8, UnLock, ((CVGA *)(getMail()->getVGA()))->getCurrentGain(), -1);

				m_AGCState = UnLock;
			}
			else // if(uNumOfClippings)
			{
				if(m_pBusInput->getSize() >= uWaitNextTime && uWaitNextTime!=0)
				{
					skipInputSamples(uWaitNextTime);
					uWaitNextTime = 0;
				}

				if(m_AGCState == UnLock)
				{
					if(m_pBusInput->getSize() > 32)
					{
						onlyClippingsCheck = false;
						pithyPass = true;

						iRMS = calculateRMS(readInputVector(32))[cpRe];
						DISPLAY("AGC  : <UnLock>rms : " << iRMS << endl);
						changeGainViaTable(iRMS);

						skipInputSamples(4);

						m_AGCState = GainUpdate;

						m_History.addEvent(m_iProcessedSamples, GainUpdate, ((CVGA *)(getMail()->getVGA()))->getCurrentGain(), iRMS);
					}
				} // if(m_AGCState == UnLock)
				else if(m_AGCState == GainUpdate)
				{
					onlyClippingsCheck = false;
					pithyPass = true;

					if(m_pBusInput->getSize() > 64)
					{
						for(iFragmentsLooked = 0; iFragmentsLooked<8; iFragmentsLooked++)
							if(checkClippings(readInputVector(4)))
							{
								m_AGCState = UnLock;
								break;
							}


					if(iFragmentsLooked == 8 && m_pBusInput->getSize() > 32)
					{
						iRMS = calculateRMS(readInputVector(32))[cpRe];
						DISPLAY("AGC  : <GainUpdate>rms : " << iRMS << endl);

						//RMSMin.setFromFlp(0.77 * 0.1768, 0, m_uPrecision, m_uExpPosition);
						//RMSMax.setFromFlp(1.22 * 0.1768, 0, m_uPrecision, m_uExpPosition);
						RMSMin[cpRe] = 69; RMSMin[cpIm] = 0;
						RMSMax[cpRe] = 110; RMSMax[cpIm] = 0;

						if ( ( RMSMin[cpRe] < iRMS ) && ( iRMS < RMSMax[cpRe] ) )
						{
							m_AGCState = Lock;
							m_History.addEvent(m_iProcessedSamples, Lock, ((CVGA *)(getMail()->getVGA()))->getCurrentGain(), iRMS);
						}
						else
						{
							changeGainViaTable(iRMS);
							m_AGCState = GainUpdate;
						}
					}
					}
				} // else if(m_AGCState == GainUpdate)
				else if(m_AGCState == Lock)
				{
					onlyClippingsCheck = false;
					pithyPass = true;

					if(m_pBusInput->getSize() > 64)
					{
						for(iFragmentsLooked = 0; iFragmentsLooked<8; iFragmentsLooked++)
							if(checkClippings(readInputVector(4)))
							{
								m_AGCState = UnLock;
								break;
							}


					if(iFragmentsLooked == 8 && m_pBusInput->getSize() > 32)
					{
						if(m_isAGCUnlock)
						{
							iRMS = calculateRMS(readInputVector(32))[cpRe];
							DISPLAY("AGC  : <Lock>rms : " << iRMS << endl);
							//RMSMin.setFromFlp(0.9 * 0.77 * 0.1768, 0, m_uConstPrecision, m_uExpPosition);
							//RMSMax.setFromFlp(1.1 * 1.22 * 0.1768, 0, m_uConstPrecision, m_uExpPosition);
							RMSMin[cpRe] = 62; RMSMin[cpIm] = 0;
							RMSMax[cpRe] = 121; RMSMax[cpIm] = 0;

							if ( ( RMSMin[cpRe] < iRMS ) && ( iRMS < RMSMax[cpRe] ) )
							{
								m_AGCState = Lock;
							}
							else
							{
								changeGainViaTable(iRMS);
								m_AGCState = GainUpdate;

								m_History.addEvent(m_iProcessedSamples, GainUpdate, ((CVGA *)(getMail()->getVGA()))->getCurrentGain(), iRMS);
							}
						}
						else
						{
							unsigned NumOfEvents = m_History.getEventNumber();
							for(unsigned i=0; i<NumOfEvents; i++)
							{
								if(m_History.getEventAGCStatus(i) == Lock && m_History.getEventSample(i) >= m_uPacketStartSample  )
								{
									m_AGCState = Freeze;
									m_uFreezeSample = m_History.getEventSample(i);
									break;
								}
							}
						}
					}
					}
				} // else if(m_AGCState == Lock)
			} // if(uNumOfClippings)
		}
		m_iClippingsStroke++;
		m_iClippingsStroke %= 4;
	} // while(!m_pBusInput->isEmpty() && m_AGCState != Freeze)

	CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);
}

void CAGC::skipInputSamples(unsigned uNumOfSamples)
{
	for(unsigned k=0; k < uNumOfSamples; k++)
		m_pBusInput->popEl();

	m_iProcessedSamples += uNumOfSamples;
}

vector<CFXPComplex16> CAGC::readInputVector(unsigned uNumOfSamples)
{
	vector<CFXPComplex16> res;
	unsigned i;

	//unsigned len = __min(uNumOfSamples, m_pBusInput->getSize()); //было
//	unsigned len = __fmin(uNumOfSamples, m_pBusInput->getSize()); //сталло в линукс
	unsigned len = fmin(uNumOfSamples, m_pBusInput->getSize()); //сталло в линукс

	for(i=0; i<len; i++)
		res.push_back(m_pBusInput->popEl());

	m_iProcessedSamples += len;

	return res;
}

vector<CFXPComplex16> CAGC::scanInputVector(unsigned uNumOfSamples)
{
	vector<CFXPComplex16> res, tmp;
	unsigned len = m_pBusInput->getSize();

	for(unsigned i=0; i<len; i++)
		tmp.push_back(m_pBusInput->popEl());
	for(unsigned i=0; i<len; i++)
		m_pBusInput->pushFrontEl(tmp[len - i - 1]);

	//len = __min(uNumOfSamples, m_pBusInput->getSize()); //было
//	len = __fmin(uNumOfSamples, m_pBusInput->getSize()); //стало
	len = fmin(uNumOfSamples, m_pBusInput->getSize()); //стало
	for(unsigned i=0; i<len; i++)
		res.push_back(tmp[tmp.size() - i - 1]);

	return res;
}

void CAGC::changeInputBus()
{
	vector<CFXPComplex16> tmp;
	unsigned len = m_pBusInput->getSize();

	for(unsigned i=0; i<len; i++)
		tmp.push_back(m_pBusInput->popEl());


	tmp[len-1][cpRe] /= 4;
	tmp[len-1][cpIm] /= 4;


	for(unsigned i=0; i<len; i++)
		m_pBusInput->pushFrontEl(tmp[len - i - 1]);
}

template <class T> vector<T> CAGC::getVectorElements(vector<T> *vData, unsigned from, unsigned to)
{
	vector<T> res;

	if((to - from > 0) && (to <= vData->size()-1))
		for(unsigned i=from; i<=to; i++)
			res.push_back(vData[i]);

	return res;
}

string CAGC::test()
{
	string tmp("CAGC::test");
	return tmp;
}

int CAGC::round(double a)
{
	return ((int)(floor(a+0.5)));
}

void CAGC::setPrecision(unsigned uPrecision)
{
	m_uPrecision = uPrecision;
}

void CAGC::setExpPosition(unsigned uExpPosition)
{
	m_uExpPosition = uExpPosition;
}

void CAGC::setConstPrecision(unsigned uConstPrecision)
{
	m_uConstPrecision = uConstPrecision;
}

void CAGC::assignGain(int iNewGain)
{
	((CVGA *)(getMail()->getVGA()))->setTagetGain(iNewGain);
}

void CAGC::addToGain(int iGainVariation)
{
	((CVGA *)(getMail()->getVGA()))->setTagetGain(
		( ((CVGA *)(getMail()->getVGA()))->getCurrentGain() + iGainVariation )
		);
}

void CAGC::initAGCAddrModLUT()
{
	double RequiredGainAddrChange;

	m_vAGCAddrModLUT.push_back(0);

	for(int i=1; i<128; i++)
	{
		RequiredGainAddrChange = 10*log10(23.0/((double)i)/0.9375);
		m_vAGCAddrModLUT.push_back(round(RequiredGainAddrChange));
	}

	m_vAGCAddrModLUT[0]=15;
	m_vAGCAddrModLUT[26]=-1; // modification
	m_vAGCAddrModLUT[27]=-1;

	DISPLAY("AGC : AGCAddrModLUT OK" << endl);
}

CFXPComplex32 CAGC::calculateRMS(vector<CFXPComplex16> vData, unsigned uSamplesNumber /* = 0*/)
{
	CFXPComplex32 RMS32(0,0);
	CFXPComplex16 RMS16(0,0);
	CFXPComplex16 con(0,0);

	unsigned uNum = 0;
	if(uSamplesNumber>0 && uSamplesNumber <= vData.size())
		uNum = uSamplesNumber;
	else
		uNum = vData.size();

	if(uNum>0)
	{
		for (unsigned i = 0; i < uNum; i++ )
		{
			RMS32[cpRe] = RMS32[cpRe] + abs(vData[i][cpRe]) + abs(vData[i][cpIm]);
		}

		switch(uNum)
		{
		case 16: RMS16 = RMS32 >> 5; break;
		case 32: RMS16 = RMS32 >> 6; break;
		case 64: RMS16 = RMS32 >> 7; break;
		}

		DISPLAY("AGC : rms = " << RMS16[cpRe] << endl);

		RMS32[cpRe] = RMS16[cpRe]*1.2533;
	}

	return RMS32;
}

void CAGC::changeGainViaTable(int RMS)
{
	int RMS_MAX_int;
	CFXPComplex16 tmp;
	int VrmsAddr;
	int iNewGain, iOldGain;

	iOldGain = ((CVGA *)(getMail()->getVGA()))->getTagetGain();

	RMS_MAX_int = 1 << m_uPrecision - 1;

//	VrmsAddr = __max((int)0, round(RMS*128.0/RMS_MAX_int)-1 ); // Compute VrmsAddr according to RMS level
//	VrmsAddr = __min(VrmsAddr, 127);
//    VrmsAddr = __fmax((int)0, round(RMS*128.0/RMS_MAX_int)-1 ); // стало
//	VrmsAddr = __fmin(VrmsAddr, 127); //стало
    VrmsAddr = fmax((int)0, round(RMS*128.0/RMS_MAX_int)-1 ); // стало
    VrmsAddr = fmin(VrmsAddr, 127); //стало

//	iNewGain = __max(0, __min(iOldGain + m_vAGCAddrModLUT[VrmsAddr],127)); //Compute changes in gain of AGC
//    iNewGain = __fmax(0, __fmin(iOldGain + m_vAGCAddrModLUT[VrmsAddr],127)); //стало
    iNewGain = fmax(0, fmin(iOldGain + m_vAGCAddrModLUT[VrmsAddr],127)); //стало

	assignGain(iNewGain);
}

unsigned CAGC::checkClippings(vector<CFXPComplex16> vData, unsigned uSamplesNumber /* = 0*/)
{
	unsigned uNumOfClippings = 0;

	unsigned uNum = 0;
	if(uSamplesNumber>0)
		uNum = uSamplesNumber;
	else
		uNum = vData.size();

	if(uNum>0)
	{
		//[ Calculate number of clippings
		for (unsigned i = 0; i < uNum; i++ )
		{
			if ( ( vData[i][cpRe] >=
				( 1 << ( m_uPrecision - 1 ) ) - 1 ) ||
				( vData[i][cpRe] <=
				-( 1 << ( m_uPrecision - 1 ) ) ) )
			{
				uNumOfClippings ++;
			}

			if ( ( vData[i][cpIm] >=
				( 1 << ( m_uPrecision - 1 ) ) - 1 ) ||
				( vData[i][cpIm] <=
				-( 1 << ( m_uPrecision - 1 ) ) ) )
			{
				uNumOfClippings ++;
			}
		}
		//]
	}

	return uNumOfClippings;
}

bool CAGC::isLockedInLastSamples(unsigned uNumOfSamples)
{
	for(unsigned i = m_iProcessedSamples - uNumOfSamples; i < m_iProcessedSamples; i++)
	{
		if(m_History.getSampleAGCStatus(i) != UnLock)
			return false;
	}

	return true;
}

void CAGC::fixPacketStartSample()
{
	m_uPacketStartSample = m_iProcessedSamples;
}
