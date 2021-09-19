#pragma once

#include "Includes.h"
#include "CBlock.h"
#include "GSLComplex/CGSLComplex.h"
#include "FXPComplex/FXPComplex16.h"

//! Class for message interchange between model blocks
class CMail
{
private:
	void *m_pADC;
	void *m_pAGC;
	void *m_pVGA;
	void *m_pAcquisitior;

public:
	CMail(void);
	~CMail(void);

	void setADC(void *pADC);
	void setAGC(void *pAGC);
	void setVGA(void *pVGA);
	void setAcquisitior(void *pAcquisitior);

	void *getADC();
	void *getAGC();
	void *getVGA();
	void *getAcquisitior();
};
