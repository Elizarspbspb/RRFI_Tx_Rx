#include "CCorrelator.h"

CCorrelator::CCorrelator(void)
{
	m_CurrentExit = CFXPComplex32(0,0);
}

CCorrelator::~CCorrelator(void)
{
}

void CCorrelator::setLength(unsigned  uLength)
{
	m_vConvolves.clear();
	m_vConvolves.setSize(uLength);

	m_uLength = uLength;

	for(unsigned i = 0; i < uLength; i++)
		m_vConvolves.addElement(CFXPComplex32(0,0));
}

CFXPComplex32 CCorrelator::doStroke(CFXPComplex16 SDirect, CFXPComplex16 SDelayed, bool isPostSquare)
{
	CFXPComplex32 res(0,0);
	CFXPComplex32 Multiply(0,0), Summ(0,0);

	if(!isPostSquare)
		Multiply = SDirect * SDelayed;
	else
	{
		Multiply[cpRe] = abs(SDirect[cpRe]) + abs(SDirect[cpIm]);
		Multiply[cpIm] = 0;
	}

	Summ[cpRe] = Multiply[cpRe] - (m_vConvolves.getFirstElement())[cpRe];
	Summ[cpIm] = Multiply[cpIm] - (m_vConvolves.getFirstElement())[cpIm];

	m_vConvolves.addElement(Multiply);

	m_CurrentExit[cpRe] = m_CurrentExit[cpRe] + Summ[cpRe];
	m_CurrentExit[cpIm] = m_CurrentExit[cpIm] + Summ[cpIm];

	if(!isPostSquare)
		res[cpRe] = abs(m_CurrentExit[cpRe]) + abs(m_CurrentExit[cpIm]);
	else
		res[cpRe] = m_CurrentExit[cpRe] * m_CurrentExit[cpRe];

	res[cpIm] = 0;
	
	return res;
}