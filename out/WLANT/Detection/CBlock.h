#pragma once
#include "CBus.h"
#include "CBeginBlock.h"
#include "CEndBlock.h"

//!Abstract class, ancestor of transit data process element (with input and output)
template <class InType, class OutType> class CBlock : public CBeginBlock<OutType>, public CEndBlock<InType>
{
	/*
	* _members:
	* CBus<T> *m_BusOutput;
	* CBus<T> *m_BusInput;
	*
	* _methods:
	* void setOutput(CBus<T> *BusOutput);
	* void setInput(CBus<T> *BusInput);
	* virtual void takeControl() = 0;
	*/
};
