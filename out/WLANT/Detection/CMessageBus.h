#pragma once
#include "Includes.h"
#include "CBus.h"
#include "CMessage.h"

class CMessageBus
{
private:
	CBus<CMessage *> BlackToWhiteBus, WhiteToBlackBus;
public:
	void sendBlack(string sMes, double dLParam, double dPParam);
	void sendWhite(string sMes, double dLParam, double dPParam);

	int receiveBlack(string &sMes, double &dLParam, double &dPParam);
	int receiveWhite(string &sMes, double &dLParam, double &dPParam);

	int receiveBlack(string &sMes, int &iLParam, int &iPParam);
	int receiveWhite(string &sMes, int &iLParam, int &iPParam);

	int getBlackInboxSize();
	int getWhiteInboxSize();

	CMessageBus(void);
	~CMessageBus(void);
};
