#include "CVGA.h"


CVGA::CVGA(void)
{
	//TRACE(_T("CAGC::CAGC\n"));
	reset();
}

CVGA::~CVGA(void)
{
	//TRACE(_T("CAGC::~CAGC\n"));
}

void CVGA::reset()
{
	m_iProcessedSamples = 0;
	setGainPerSample(exp( 1.0 / 4.0 ));
	m_iCurrentGain = 127;
	setTagetGain(127);
}

string CVGA::test()
{
	string tmp("CVGA::test");
	return tmp;
}

void CVGA::takeControl()
{
	double gain_change_per_sample;
	double gain_factor; 
	char up = 0;
	CGSLComplex CurSample(0,0);

	if ( m_iCurrentGain > m_iTagetGain )
	{
		gain_change_per_sample = 1.0 / m_dGainChangePerSample;
		up = -1;
	}
	else if ( m_iCurrentGain < m_iTagetGain )
	{
		gain_change_per_sample = m_dGainChangePerSample;
		up = 1;
	}

	while(!m_pBusInput->isEmpty())
	{
		gain_factor = exp( log(10.0) * 0.9375 * m_iCurrentGain / 10.0 );

		CurSample = m_pBusInput->popEl();

		DISPLAY("To   VGA : " << CurSample.real() << " " << CurSample.imag() << endl);

		CurSample.imag(CurSample.imag()*gain_factor);
		CurSample.real(CurSample.real()*gain_factor);

		DISPLAY("From VGA : " << CurSample.real() << " " << CurSample.imag() << endl);

		m_pBusOutput->pushEl(CurSample);

		if ( m_iCurrentGain != m_iTagetGain )
		{
			m_iCurrentGain = (int)floor(((double)m_iCurrentGain) * gain_change_per_sample);
		}

		if ( ( up == -1 ) && ( m_iCurrentGain <= m_iTagetGain ) )
		{
			m_iCurrentGain = m_iTagetGain;
		}

		if ( ( up == 1 ) && ( m_iCurrentGain >= m_iTagetGain ) )
		{
			m_iCurrentGain = m_iTagetGain;
		}

		m_iProcessedSamples++;
	}
}