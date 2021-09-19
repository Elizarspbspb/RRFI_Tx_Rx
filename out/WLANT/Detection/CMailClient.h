#pragma once

#include "CMail.h"
#include "Includes.h"

class CMailClient
{
public:
	CMailClient(void)	{};
	virtual ~CMailClient(void)	{};

	void setMail(CMail *pMail)
	{
		m_pMail = pMail;
	};

	CMail *getMail()
	{
		return m_pMail;
	};

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	CMail *m_pMail;
};
