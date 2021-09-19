#pragma once

#include "Includes.h"
#include "CBlock.h"
#include "FXPComplex/FXPComplex16.h"
#include "GSLComplex/CGSLComplex.h"

#include "CMailClient.h"
#include "CMail.h"

///Class for float-point signal quantization to fxp-signal
class CADC : public CBlock<CGSLComplex, CFXPComplex16>, public CMailClient
{
public:
	CADC(void);
	~CADC(void);

	void takeControl();
	void setLinearRoutine();
	void setTableRoutine();

	string test();

	void setPrecision(unsigned uPrecision);
	void setConverterMax(int iConverterMax);
	void setExpPosition(unsigned uExpPosition);

/////////////////////////////////////////----- private -----//////////////////////////////////////////////
private:
	enum routineTypes {linear, table};
	routineTypes m_routineType;

	unsigned m_uPrecision;
	unsigned m_uExpPosition;  
	int m_iConverterMax;

	/// Total number of processed samples
	unsigned m_iProcessedSamples;

private:
	/// Table for non-linear ADC
	vector<double> m_vADCTable;
	bool m_isADCTableInit;

private:
	CFXPComplex16 routineADCLinear(CGSLComplex value);
	CFXPComplex16 routineADCTable(CGSLComplex value);

	void initADCTable();
};