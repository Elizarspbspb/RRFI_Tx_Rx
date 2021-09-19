#pragma once
#include "Includes.h"

class CMessage
{
private:
	string m_sMes;
	double m_dLParam;
	double m_dPParam;
public:
	CMessage();
	CMessage(string sMes, double dLParam, double dPParam);
	void setMessage(string sMes, double dLParam, double dPParam);
	void getMessage(string &sMes, double &dLParam, double &dPParam);
	~CMessage();
};
