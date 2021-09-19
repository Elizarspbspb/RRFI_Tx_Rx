#pragma once
#include "CBus.h"

//!Abstract class, ancestor of any data process element
template <class T> class CBeginBlock
{
public:
	CBeginBlock(){};
	virtual ~CBeginBlock(){};

	void setOutput(CBus<T> *pBusOutput) 
	{ 
		m_pBusOutput = pBusOutput; 
	};
	virtual void takeControl() = 0;
	virtual string test() = 0;

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
protected:
	CBus<T> *m_pBusOutput;

};