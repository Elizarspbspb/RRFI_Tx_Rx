#include "CADC.h"


CADC::CADC(void)
{
	//TRACE(_T("CADC::CADC\n"));
	m_iProcessedSamples = 0;
	m_isADCTableInit = false;
	m_routineType = linear;
}

CADC::~CADC(void)
{
	//TRACE(_T("CADC::~CADC\n"));
}

void CADC::takeControl()
{
	CFXPComplex16 CurSample;
	while(!m_pBusInput->isEmpty())
	{
		DISPLAY("To ADC   : " << m_pBusInput->getFrontEl().real() << " " << m_pBusInput->getFrontEl().imag() << endl);

		if(m_routineType == linear)
			CurSample = routineADCLinear(m_pBusInput->popEl());

		if(m_routineType == table)
			CurSample = routineADCTable(m_pBusInput->popEl());

		DISPLAY("From ADC : " << CurSample[cpRe] << " " << CurSample[cpIm] << endl);

		m_pBusOutput->pushEl(CurSample);

		m_iProcessedSamples++;
	}
}

void CADC::setLinearRoutine()
{
	m_routineType = linear;
}

void CADC::setTableRoutine()
{
	m_routineType = table;
	if(!m_isADCTableInit)
		initADCTable();
}

string CADC::test()
{
	string tmp("CADC::test");
	return tmp;
}

void CADC::setPrecision(unsigned uPrecision)
{
	m_uPrecision = uPrecision;
}
void CADC::setExpPosition(unsigned uExpPosition)
{
	m_uExpPosition = uExpPosition;
}

void CADC::setConverterMax(int iConverterMax)
{
	m_iConverterMax = iConverterMax; 
}

CFXPComplex16 CADC::routineADCLinear(CGSLComplex value)
{
	CFXPComplex16 res;
	double part;

	int N = 1 << m_uPrecision;
	part = value.real();
	if ( part >= m_iConverterMax ){
		res[cpRe] =  (N/2 - 1);
	}
	else if ( part < -m_iConverterMax ){
		res[cpRe] =  (-N / 2);
	}
	else {
		res[cpRe] =  (int)(
			floor( ( part + m_iConverterMax ) * N /
			( 2 * m_iConverterMax ) ) - N / 2
			);
	}

	part = value.imag();
	if ( part >= m_iConverterMax ){
		res[cpIm] =  (N/2 - 1);
	}
	else if ( part < -m_iConverterMax ){
		res[cpIm] =  (-N / 2);
	}
	else {
		res[cpIm] =  (int)(
			floor( ( part + m_iConverterMax ) * N /
			( 2 * m_iConverterMax ) ) - N / 2
			);
	}

	return res;
}

CFXPComplex16 CADC::routineADCTable(CGSLComplex value)
{
	CFXPComplex16 res;
	double part;

	int l,u;
	int m;
	int N;
	int r;

	N= 1 << m_uPrecision;
	l = 0;
	u = N-1;
	part = value.real();
	if ( part >= m_vADCTable[u] ){
		r=u;
	}
	else if ( part < m_vADCTable[l] ){
		r=l;
	}
	else {
		while(l<u-1){
			m = (l+u)/2;
			if(part < m_vADCTable[m]){
				u=m;
			} else {
				l=m;
			}
		}
		r=l;
	}
	res[cpRe] = r - (N/2);


	N= 1 << m_uPrecision;
	l = 0;
	u = N-1;
	part = value.imag();
	if ( part >= m_vADCTable[u] ){
		r=u;
	}
	else if ( part < m_vADCTable[l] ){
		r=l;
	}
	else {
		while(l<u-1){
			m = (l+u)/2;
			if(part < m_vADCTable[m]){
				u=m;
			} else {
				l=m;
			}
		}
		r=l;
	}
	res[cpIm] = r - (N/2);

	return res;
}

void CADC::initADCTable()
{
	int i;
	int N;
	double d;
	double s;

	N = 1 << m_uPrecision;

	s = -m_iConverterMax;
	d = 2.0*m_iConverterMax / N;

	for(i=0;i<N;i++){
		m_vADCTable.push_back(s+i*d);
	}

	m_isADCTableInit = true;
}