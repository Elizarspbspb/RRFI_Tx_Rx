#pragma once

//!Class, containing model parameters
class CModelParameters
{
public:
	unsigned m_uPrecision;
	unsigned m_uExpPosition;

	int m_iADCMax;

	unsigned m_uAcquisitiorDecimationRate;
	
	unsigned m_uCorrelationLength;
	unsigned m_uCorrelationDelay;

	unsigned m_uAcquisitiorWindowSize;

	unsigned m_uAGCPrecision;

public:
	CModelParameters(void) 	
	{
		m_uPrecision = 12;
		m_uExpPosition = 11;
		m_uAcquisitiorDecimationRate = 1;
		m_iADCMax = 1;

		m_uCorrelationDelay = 64; 
		m_uCorrelationLength = 48; 

		m_uAcquisitiorWindowSize = 150;

		m_uAGCPrecision = 16;
	};

	CModelParameters(unsigned uPrecision, unsigned uExpPosition, unsigned uAcquisitiorDecimationRate, int iADCMax,
		unsigned uCorrelationLength, unsigned uCorrelationDelay, unsigned uAcquisitiorWindowSize) 	
	{
		setParameters(uPrecision, uExpPosition, uAcquisitiorDecimationRate, iADCMax, 
			uCorrelationLength, uCorrelationDelay, uAcquisitiorWindowSize);
	};

	void setParameters(unsigned uPrecision, unsigned uExpPosition, unsigned uAcquisitiorDecimationRate, int iADCMax,
		unsigned uCorrelationLength, unsigned uCorrelationDelay, unsigned uAcquisitiorWindowSize)
	{
		m_uPrecision = uPrecision;
		m_uExpPosition = uExpPosition;
		m_uAcquisitiorDecimationRate = uAcquisitiorDecimationRate;
		m_iADCMax = iADCMax;
		m_uCorrelationLength = uCorrelationLength;
		m_uCorrelationDelay = uCorrelationDelay;
		m_uAcquisitiorWindowSize = uAcquisitiorWindowSize;
	};

	~CModelParameters(void)	{	};
};
