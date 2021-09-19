#include "CAcquisitior.h"

CAcquisitior::CAcquisitior(void)
{
	//TRACE(_T("CAcquisitior::CAcquisitior\n"));

	m_iProcessedSamples = 0;
	m_isLogging = false;
	m_isPacketFound = false;
	iTh_exceedings = m_iSleep = iTh_exceedingsW = 0;
}

CAcquisitior::~CAcquisitior(void)
{
	//TRACE(_T("CAcquisitior::~CAcquisitior\n"));\

	m_DESLog.close();
}

void CAcquisitior::setDESLog(string filename)
{
	m_DESLog.open(filename.c_str());
	m_isLogging = true;
}

void CAcquisitior::setPrecision(unsigned uPrecision)
{
	m_uPrecision = uPrecision;
}

void CAcquisitior::setExpPosition(unsigned uExpPosition)
{
	m_uExpPosition = uExpPosition;
}

void CAcquisitior::setWindowSize(unsigned uWindowSize)
{
	m_uWindowSize = uWindowSize;
	m_vWindowResponse.setSize(uWindowSize);
	m_vWindowEnergy.setSize(uWindowSize);
}

void CAcquisitior::takeControl()
{
	//TRACE(_T("CAcquisitior::takeControl\n"));

	CFXPComplex32 Response, Energy, Energy2, EnergyT;
	CFXPComplex32 ResponseEx, EnergyEx;
	CFXPComplex16 Response16, Energy16;
	CFXPComplex16 ConjSample(0,0), CurSample(0,0);

	CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY);

	while(!m_pBusInput->isEmpty())
	{
		CurSample = m_pBusInput->popEl();

		ConjSample[cpRe] = CurSample[cpRe];
		ConjSample[cpIm] = - CurSample[cpIm];
		Energy = m_EnergyCorrelator.doStroke(CurSample, ConjSample, false);
		

		if(m_iProcessedSamples > m_uDelay)
			Response = m_ResponceCorrelator.doStroke(m_vStorageLine.getFirstElement(), ConjSample, false);
	
		m_vStorageLine.addElement(CurSample);

		m_iProcessedSamples++;
		m_iSleep++;

		//Calculate Autocorrelation and energy
		
		DISPLAY("Acquisitior : Responce32 = re " << Response[cpRe] << "; im " << Response[cpIm] << endl);
		DISPLAY("Acquisitior : Energy32   = re " << Energy[cpRe] << "; im " << Energy[cpIm] << endl);

		// ////////////////////////////////////////////////////////////////////////
		// Analise correlation response, detect packet location
		// ////////////////////////////////////////////////////////////////////////
		
		//Response16 = Response >> m_uResponseShift;;
		//Energy16 = Energy >> m_uEnergyShift;

		//DISPLAY("Acquisitior : Responce16 = re " << Response16[cpRe] << "; im " << Response16[cpIm] << endl);
		//DISPLAY("Acquisitior : Energy16   = re " << Energy16[cpRe] << "; im " << Energy16[cpIm] << endl);

		// Logging
		if(m_isLogging)
		{
			if((double)Energy[cpRe]!=0)
			{
				m_DESLog << m_iProcessedSamples << " " ;
				m_DESLog << ((double)Response[cpRe])/((double)Energy[cpRe]) << endl;
			}
			else
				m_DESLog << 0 << endl;
		}

		//Energy[cpRe] = Energy16[cpRe] ; Energy[cpIm] = 0;
		//Response[cpRe] = Response16[cpRe]; Response[cpIm] = 0;

		EnergyT[cpRe] = Energy[cpRe] * 0.15; EnergyT[cpIm] = 0;
		Energy2[cpRe] = Energy[cpRe] * 0.4; Energy2[cpIm] = 0;
		Energy[cpRe] = Energy[cpRe] * 0.2; Energy[cpIm] = 0;
		Response[cpRe] = Response[cpRe]; Response[cpIm] = 0;

		// Fill window
		m_vWindowResponse.addElement(Response);
		m_vWindowEnergy.addElement(EnergyT);

		if(Response[cpRe] > Energy[cpRe])
			iTh_exceedings++;
		else
			iTh_exceedings = 0;

		if(Response[cpRe] > Energy2[cpRe])
			iTh_exceedings2++;
		else
			iTh_exceedings2 = 0;

		if(Response[cpRe] > EnergyT[cpRe])
			iTh_exceedingsW++;
		if(m_vWindowResponse.getCurrentSize() == m_uWindowSize && m_vWindowEnergy.getCurrentSize() == m_uWindowSize)
			if(m_vWindowResponse.getFirstElement()[cpRe] > m_vWindowEnergy.getFirstElement()[cpRe])
				iTh_exceedingsW--;

		if((iTh_exceedings > 200 || (iTh_exceedings2 > 15 && iTh_exceedingsW > 130)) && m_iSleep > 0 )
		{
			// TODO : hoist packet detection flag
			m_isPacketFound = true;
			((CAGC *)(getMail()->getAGC()))->fixPacketStartSample();

			DISPLAYX("packet found at " << m_iProcessedSamples << endl);
			DISPLAYX(" - current VGA : " << ((CVGA *)getMail()->getVGA())->getCurrentGain() << endl);
			DISPLAYX(" - current WinWeight : " << iTh_exceedingsW << endl);

			iTh_exceedings = 0;
			m_iSleep = -500;
		}

		// ////////////////////////////////////////////////////////////////////////
	}

	CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);
}

string CAcquisitior::test()
{
	string tmp("CAcquisitior::test");
	return tmp;
}

bool CAcquisitior::isPacketFound()
{
	return m_isPacketFound;
}

void CAcquisitior::init(unsigned uDelay, unsigned uAutoCorrelationLength)
{
	setAutoCorrelationDelay(uDelay);
	setAutoCorrelationLength(uAutoCorrelationLength);
}

void CAcquisitior::setAutoCorrelationDelay(unsigned uDelay)
{
	m_uDelay = uDelay;
	m_vStorageLine.setSize(uDelay);
}

void CAcquisitior::setAutoCorrelationLength(unsigned uAutoCorrelationLength)
{
	m_uAutoCorrelationLength = uAutoCorrelationLength;

	m_uResponseShift = (unsigned)(log((double)uAutoCorrelationLength) / log((double)2) + 1) + 3;
	m_uEnergyShift = (unsigned)(2*(log((double)uAutoCorrelationLength) / log((double)2)) + 2) + 4;

	m_vDelayedLine.setSize(uAutoCorrelationLength);
	m_vDirectLine.setSize(uAutoCorrelationLength);

	m_ResponceCorrelator.setLength(uAutoCorrelationLength);
	m_EnergyCorrelator.setLength(uAutoCorrelationLength);
}

CFXPComplex32 CAcquisitior::calculateAutoCorrelation(vector<CFXPComplex16> vSampling, unsigned uDelay, unsigned uLength)
{
	CFXPComplex32 Response(0,0);

	if( ((uDelay+uLength)<vSampling.size()) )
		return Response;

	for(unsigned i=uDelay; i<(uDelay+uLength); i++)
	{
		Response = Response + (vSampling[i]^vSampling[i-uDelay]);
	}

	Response = CFXPComplex16::addnsub(abs(Response[cpRe]), 0, abs(Response[cpIm]), 0);
	
	return Response;
}


CFXPComplex32 CAcquisitior::calculateAutoCorrelation(vector<CFXPComplex16> vSampling1, vector<CFXPComplex16> vSampling2)
{
	CFXPComplex32 Response(0,0);

	if(vSampling1.size() != vSampling2.size())
		return Response;

	for(unsigned i=0; i<vSampling1.size(); i++)
	{
		Response = Response + (vSampling1[i]^vSampling2[i]);
	}

	Response[cpRe] = abs(Response[cpRe]) + abs(Response[cpIm]);
	Response[cpIm] = 0;

	return Response;
}

CFXPComplex32 CAcquisitior::calculateEnergy(vector<CFXPComplex16> vSampling, unsigned uBegin, unsigned uEnd)
{
	CFXPComplex32 Energy(0,0);

	if((uBegin>=uEnd) || (uBegin>=vSampling.size()) || (uEnd>vSampling.size()))
		return Energy;

	/*
	for(unsigned i=uBegin; i<uEnd; i++)
	{	
		Energy[cpRe] = Energy[cpRe] + abs(vSampling[i][cpRe]) + abs(vSampling[i][cpIm]);
	}

	Energy[cpRe] = Energy[cpRe] * Energy[cpRe];
	*/

	for(unsigned i=uBegin; i<uEnd; i++)
	{	
		Energy = Energy + (vSampling[i]^vSampling[i]);
	}

	Energy[cpRe] = abs(Energy[cpRe]) + abs(Energy[cpIm]);
	Energy[cpIm] = 0;

	return Energy;
}

CFXPComplex32 CAcquisitior::calculateEnergy(vector<CFXPComplex16> vSampling)
{
	return calculateEnergy(vSampling, 0, vSampling.size());
}