#pragma once

#include "Includes.h"
#include "CBlock.h"

//! Class for signal decimation
template <class T> class CDecimator : public CBlock<T, T>
{
public:
	CDecimator() : CBlock<T, T>()
	{
		reset(1);
	};

	CDecimator(int iDecimationRate) : CBlock<T, T>()
	{
		reset(iDecimationRate);
	};

	~CDecimator(void){};

	void reset(int iDecimationRate)
	{
		m_iDecimationRate = iDecimationRate;
		m_iStroke = 0;
	};

	void takeControl()
	{
		//while(!m_pBusInput->isEmpty()) //было
		/*while(!m_vBusOutput->isEmpty()) //стало
		{
			m_iStroke %= m_iDecimationRate;

			DISPLAY("Decimator :  stroke " << m_iStroke << "/" << m_iDecimationRate);

			if(m_iStroke == 0)
			{
				//m_pBusOutput->pushEl(m_pBusInput->getFrontEl()); //было
				m_vBusOutput->pushEl(m_vBusOutput->getFrontEl()); //стало
				DISPLAY(" <pass>");
			}

			DISPLAY(endl);

			//m_pBusInput->popEl();
			m_vBusOutput->popEl();

			m_iStroke++;
		}*/
	};

	string test()
	{
		string tmp("CDecimator::test");
		return tmp;
	};

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	int m_iDecimationRate;
	int m_iStroke;
};
