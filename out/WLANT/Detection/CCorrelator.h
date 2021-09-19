#pragma once

#include "Includes.h"
#include "FXPComplex/FXPComplex16.h"
#include "FXPComplex/FXPComplex32.h"
#include "CThroughVector.h"

class CCorrelator
{
public:
	CCorrelator(void);
	~CCorrelator(void);

	void setLength(unsigned  uLength);
	CFXPComplex32 doStroke(CFXPComplex16 SDirect, CFXPComplex16 SDelayed, bool isPostSquare);

private:
	CThroughVector<CFXPComplex32> m_vConvolves;
	CFXPComplex32 m_CurrentExit;

	unsigned m_uLength;
};	

