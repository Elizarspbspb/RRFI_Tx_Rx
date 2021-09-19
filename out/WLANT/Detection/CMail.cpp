#include "CMail.h"

CMail::CMail(void)
{
}

CMail::~CMail(void)
{
}

void CMail::setADC(void *pADC)
{
	m_pADC = pADC;
}
void CMail::setAGC(void *pAGC)
{
	m_pAGC = pAGC;
}
void CMail::setVGA(void *pVGA)
{
	m_pVGA = pVGA;
}
void CMail::setAcquisitior(void *pAcquisitior)
{
	m_pAcquisitior = pAcquisitior;
}

void *CMail::getADC()
{
	return m_pADC;
}
void *CMail::getAGC()
{
	return m_pAGC;
}
void *CMail::getVGA()
{
	return m_pVGA;
}
void *CMail::getAcquisitior()
{
	return m_pAcquisitior;
}