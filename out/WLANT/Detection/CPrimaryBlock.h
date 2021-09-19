#pragma once
#include "CBus.h"

//!Abstract class, ancestor of any data process element
template <class T> class CPrimaryBlock
{
	CBus<T> *m_BusInput;
public:
	CPrimaryBlock(){};
	~CPrimaryBlock(){};

	void setInput(CBus<T> *BusInput) { m_BusInput = BusInput; };
	virtual void takeControl() = 0;
};