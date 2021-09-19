#pragma once
#include "CBus.h"

//!Abstract class, ancestor of any data process element
template <class T> class CEndBlock
{
public:
	CEndBlock(){};
	virtual ~CEndBlock(){};

	void setInput(CBus<T> *pBusInput) 
	{
		m_pBusInput = pBusInput;
	};
	virtual void takeControl() = 0;
	virtual string test() = 0;

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
protected:
	CBus<T> *m_pBusInput;
};