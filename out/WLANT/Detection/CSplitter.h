#pragma once

#include "Includes.h"
#include "CEndBlock.h"

//!Class with one input bus and with set of output chanells
template <class T> class CSplitter : public CEndBlock<T>
{
public:
	CSplitter() : CEndBlock<T>(){};
	~CSplitter() { m_vBusOutput.clear(); };

	void registerBusOutput(CBus<T> *newBus) { m_vBusOutput.push_back(newBus); };

	void takeControl()
	{

		//while(!m_pBusInput->isEmpty()) //было
		//while(!m_vBusOutput->isEmpty()) //стало
		/*while(!m_vBusOutput->isEmpty()) //стало
		{
			DISPLAY("To Splitter : " << m_pBusInput->getFrontEl()[cpRe] << " " << m_pBusInput->getFrontEl()[cpIm] << endl);

			for(unsigned int i=0; i<m_vBusOutput.size(); i++)
					//m_vBusOutput[i]->pushEl(m_pBusInput->getFrontEl()); //была
					//m_vBusOutput[i]->pushEl(m_vBusOutput->getFrontEl()); //стало закоментил тиак как ошибки


			//m_pBusInput->popEl(); //было
			//m_vBusOutput.popEl(); //стало закоментиил так как ошибки
		}*/
	};

	int getSize()
	{
		return m_vBusOutput.size();
	};

	string test()
	{
		string tmp("CSplitter::test");
		return tmp;
	}

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	vector<CBus<T> *> m_vBusOutput;
};
