#pragma once
#include "CBus.h"

//!Abstract class, ancestor of any data process element
template <class T> class CFinallyBlock
{
	CBus<T> *m_BusOutput;
public:
	CFinallyBlock(){};
	~CFinallyBlock(){};

	void setOutput(CBus<T> *BusOutput) { m_BusOutput = BusOutput; };
	virtual void takeControl() = 0;
};