#include "DecimatorFilter.h"

CDecimatorFilter::CDecimatorFilter() : CBlock<CFXPComplex16, CFXPComplex16>()
{
	m_filter2[0]	=  0.001532562f;
	m_filter2[1]	= -0.001674060f;
	m_filter2[2]	=  0.002554935f;
	m_filter2[3]	= -0.003713823f;
	m_filter2[4]	=  0.005213502f;
	m_filter2[5]	= -0.007128707f;
	m_filter2[6]	=  0.009565063f;
	m_filter2[7]	= -0.012676025f;
	m_filter2[8]	=  0.016697756f;
	m_filter2[9]	= -0.022044785;
	m_filter2[10]	=  0.029492361f;
	m_filter2[11]	= -0.040713548f;
	m_filter2[12]	=  0.060082826f;
	m_filter2[13]	= -0.103688115f;
	m_filter2[14]	=  0.316500058f;
	m_filter2[15]	=  0.316500058f;
	m_filter2[16]	= -0.103688115f;
	m_filter2[17]	=  0.060082826f;
	m_filter2[18]	= -0.040713548f;
	m_filter2[19]	=  0.029492361f;
	m_filter2[20]	= -0.022044785f;
	m_filter2[21]	=  0.016697756f;
	m_filter2[22]	= -0.012676025f;
	m_filter2[23]	=  0.009565063f;
	m_filter2[24]	= -0.007128707f;
	m_filter2[25]	=  0.005213502f;
	m_filter2[26]	= -0.003713823f;
	m_filter2[27]	=  0.002554935f;
	m_filter2[28]	= -0.001674060f;
	m_filter2[29]	=  0.001532562f;

	m_iFilterLength = 30;
	m_iFilterHalfLength = m_iFilterLength / 2;

	m_uProcessedSamples = 0;

	mi1 = mi2 = mi3 = mi4 = 0;

	reset(1);
};

CDecimatorFilter::CDecimatorFilter(int iDecimationRate) : CBlock<CFXPComplex16, CFXPComplex16>()
{
	reset(iDecimationRate);
};

void CDecimatorFilter::reset(int iDecimationRate)
{
	m_iDecimationRate = iDecimationRate;
	m_iStroke = 0;
};

void CDecimatorFilter::takeControl()
{
	//while(!m_pBusInput->isEmpty())
	{
		int i, j;
		if(m_pBusInput->getSize() > 87 && m_uProcessedSamples == 0)
		{
			for( i = 0; i < m_iFilterLength; i++)
				m_vInput.push_back(m_pBusInput->popEl());

			m_uProcessedSamples += m_iFilterLength;

			for( i = 0; i < m_iFilterHalfLength; i++ )	
				m_vBuffer.push_back(CFXPComplex16(m_vInput[2*i][cpRe] * 0.5, m_vInput[2*i][cpIm] * 0.5));

			for( i = 0; i < m_iFilterHalfLength; i++ )	{
				for( j = 0; j < m_iFilterHalfLength + i; j++ )	{
					if(m_vBuffer.size() >= m_iFilterHalfLength && m_vInput.size() >= 1+m_iFilterLength) {
						m_vBuffer[i][cpRe] += m_vInput[1+2*j][cpRe]*m_filter2[m_iFilterHalfLength - 1 + i-j];
						m_vBuffer[i][cpIm] += m_vInput[1+2*j][cpIm]*m_filter2[m_iFilterHalfLength - 1 + i-j];
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////
			for(unsigned k=0; k < m_vBuffer.size(); k+=2)
				m_pBusOutput->pushEl(m_vBuffer[k]);
			//////////////////////////////////////////////////////////////////////////
			m_vBuffer.clear();
			m_iSkipEl = 1;

			while(m_vInput.size() < 87)
				m_vInput.push_back(m_pBusInput->popEl());
		}


		if(m_pBusInput->getSize() > m_iFilterLength && m_uProcessedSamples != 0)
		{

			for(i = m_iFilterHalfLength; i < m_iFilterLength; i++ )
			{
				m_vInput.push_back(m_pBusInput->popEl());
				m_vInput.push_back(m_pBusInput->popEl());
			}

			//for(int k=0; k < m_vInput.size(); k++)
			//	cout <<"input : " <<m_vInput[k][cpRe] << endl;

			m_uProcessedSamples += m_iFilterLength;

			for(i = m_iFilterHalfLength; i < m_iFilterLength && (m_vInput.size() >= 2*(m_iFilterLength-m_iFilterHalfLength-1)); i++ )
				m_vBuffer.push_back(CFXPComplex16(m_vInput[2*i][cpRe] * 0.5, m_vInput[2*i][cpIm] * 0.5));

			//for(int k=0; k < m_vBuffer.size(); k++)
			//	cout << "buffer* : " << (m_vBuffer[k])[cpRe] << endl;

			for( i = m_iFilterHalfLength; i < m_iFilterLength; i++ )	{
				for(j = 0; j < m_iFilterLength; j++ )	{
					if(m_vInput.size() >= (2*(i + j) - m_iFilterLength + 1) && m_vBuffer.size() >= m_iFilterHalfLength)
					{
						m_vBuffer[i-m_iFilterHalfLength][cpRe] += m_vInput[2*(i + j) - m_iFilterLength + 1][cpRe] * m_filter2[m_iFilterLength - 1 - j];
						m_vBuffer[i-m_iFilterHalfLength][cpIm] += m_vInput[2*(i + j) - m_iFilterLength + 1][cpIm] * m_filter2[m_iFilterLength - 1 - j];
					}
				}
			}
			
			//////////////////////////////////////////////////////////////////////////
			if(m_iSkipEl == 1)
			{
				for(unsigned k=0; k < m_vBuffer.size()-2; k+=2)
				{
					m_pBusOutput->pushEl(m_vBuffer[k+1]);
					//cout << "output* : " << m_vBuffer[k+1][cpRe] << endl;
				}
				m_iSkipEl = 0;
			}
			else
			{
				for(unsigned k=0; k < m_vBuffer.size(); k+=2)
				{
					m_pBusOutput->pushEl(m_vBuffer[k]);
					//cout << "output* : " << m_vBuffer[k][cpRe] << endl;
				}
				m_iSkipEl = 1;
			}
			//cout << endl;
			//////////////////////////////////////////////////////////////////////////

			m_vBuffer.clear();

			for(i = m_iFilterHalfLength; i < m_iFilterLength; i++ )
			{
				m_vInput.erase(m_vInput.begin());
				m_vInput.erase(m_vInput.begin());
			}
			
		}
	}
};

string CDecimatorFilter::test()
{
	string tmp("CDecimatorFilter::test");
	return tmp;
};