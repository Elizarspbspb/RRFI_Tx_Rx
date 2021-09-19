#pragma once

#include "Includes.h"

class CAGCHistory
{
public:

	CAGCHistory(void)
	{
	}

	~CAGCHistory(void)
	{
	}

	void addEvent(unsigned uSample, int iAGCStatus, int iGain, int iRMS)
	{
		AGCEvent newEvent;

		newEvent.sample = uSample;
		newEvent.AGCStatus = iAGCStatus;
		newEvent.gain = iGain;
		newEvent.RMS = iRMS;

		m_vEventHistory.push_back(newEvent);
	}

	unsigned getEventNumber()
	{
		return m_vEventHistory.size();
	}

	unsigned getEventSample(unsigned uNumber)
	{
		if(uNumber <= m_vEventHistory.size())
			return m_vEventHistory[uNumber].sample;
		else
			return 0;
	};

	int getEventAGCStatus(unsigned uNumber)
	{
		if(uNumber <= m_vEventHistory.size())
			return m_vEventHistory[uNumber].AGCStatus;
		else
			return 0;
	};

	int getSampleAGCStatus(unsigned uSampleNum)
	{
		unsigned i = 0;

		while ( i < m_vEventHistory.size() )
		{
			if ( m_vEventHistory[i].sample >= uSampleNum )
				break;
			i++;
		}

		if ( i == m_vEventHistory.size() )
			return m_vEventHistory[m_vEventHistory.size() - 1].AGCStatus;
		else
			return m_vEventHistory[ i == 0 ? i : (i - 1) ].AGCStatus;

	};

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	struct AGCEvent
	{
		unsigned sample;
		int AGCStatus;
		int gain;
		int RMS;
	};

public:
	vector<AGCEvent> m_vEventHistory;
};
