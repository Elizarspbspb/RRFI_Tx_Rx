//#include "cmessage.h"
#include "CMessage.h"

CMessage::CMessage()
{
	m_sMes = "";
	m_dLParam = 0;
	m_dPParam = 0;
}

CMessage::CMessage(string sMes, double dLParam, double dPParam)
{
	m_sMes = sMes;
	m_dLParam = dLParam;
	m_dPParam = dPParam;
}

CMessage::~CMessage()
{
}

void CMessage::setMessage(string sMes, double dLParam, double dPParam)
{
	m_sMes = sMes;
	m_dLParam = dLParam;
	m_dPParam = dPParam;
}
void CMessage::getMessage(string &sMes, double &dLParam, double &dPParam)
{
	sMes = m_sMes;
	dLParam = m_dLParam;
	dPParam = m_dPParam;
}
