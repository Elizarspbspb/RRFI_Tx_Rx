#include "CMessageBus.h"

CMessageBus::CMessageBus(void)
{
}

CMessageBus::~CMessageBus(void)
{
}

void CMessageBus::sendBlack(string sMes, double dLParam, double dPParam)
{
	BlackToWhiteBus.pushEl(new CMessage(sMes, dLParam, dPParam));
}

void CMessageBus::sendWhite(string sMes, double dLParam, double dPParam)
{
	WhiteToBlackBus.pushEl(new CMessage(sMes, dLParam, dPParam));
}

int CMessageBus::receiveBlack(string &sMes, double &dLParam, double &dPParam)
{
	if(!WhiteToBlackBus.isEmpty())
	{
		CMessage *Message = new CMessage;
		Message = WhiteToBlackBus.popEl();
		Message->getMessage(sMes, dLParam, dPParam);
		delete Message;
		return 1;
	}
	else
		return 0;

}

int CMessageBus::receiveWhite(string &sMes, double &dLParam, double &dPParam)
{
	if(!BlackToWhiteBus.isEmpty())
	{
		CMessage *Message = new CMessage;
		Message = BlackToWhiteBus.popEl();
		Message->getMessage(sMes, dLParam, dPParam);
		delete Message;
		return 1;
	}
	else
		return 0;
}

int CMessageBus::receiveBlack(string &sMes, int &iLParam, int &iPParam)
{
	if(!WhiteToBlackBus.isEmpty())
	{
		CMessage *Message = new CMessage;
		Message = WhiteToBlackBus.popEl();
		double d1, d2;
		Message->getMessage(sMes, d1, d2);
		iLParam = (int)d1;
		iPParam = (int)d2;
		delete Message;
		return 1;
	}
	else
		return 0;
}
int CMessageBus::receiveWhite(string &sMes, int &iLParam, int &iPParam)
{
	if(!BlackToWhiteBus.isEmpty())
	{
		CMessage *Message = new CMessage;
		Message = BlackToWhiteBus.popEl();
		double d1, d2;
		Message->getMessage(sMes, d1, d2);
		iLParam = (int)d1;
		iPParam = (int)d2;
		delete Message;
		return 1;
	}
	else
		return 0;
}

int CMessageBus::getBlackInboxSize()
{
	return WhiteToBlackBus.getSize();
}
int CMessageBus::getWhiteInboxSize()
{
	return BlackToWhiteBus.getSize();
}
