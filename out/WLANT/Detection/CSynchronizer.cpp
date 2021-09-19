#include "CSynchronizer.h"

CSynchronizer::CSynchronizer(void)
{
	//TRACE(_T("CSynchronizer::CSynchronizer\n"));
}

CSynchronizer::~CSynchronizer(void)
{
	//TRACE(_T("CSynchronizer::~CSynchronizer\n"));
}

void CSynchronizer::initSynchronizer(bool isFileInput /* = true */, gsl_complex *pSignal /* = NULL*/)
{
	//TRACE(_T("CSynchronizer::initSynchronizer\n"));

	//Create system
	m_pReceiver = new CReceiver;
	m_pMail = new CMail;
	m_pSplitter = new CSplitter<CFXPComplex16>();
	m_pDecimator = new CDecimator<CFXPComplex16>();
	//m_pDecimator = new CDecimatorFilter;
	m_pModelParameters = new CModelParameters;

	//Create model
	m_pADC = new CADC;
	m_pAGC = new CAGC;
	m_pVGA = new CVGA;
	m_pAcquisitior = new CAcquisitior;

	//Initialize system
	m_pMail->setADC(m_pADC);
	m_pMail->setAGC(m_pAGC);
	m_pMail->setVGA(m_pVGA);
	m_pMail->setAcquisitior(m_pAcquisitior);
	m_pDecimator->reset(m_pModelParameters->m_uAcquisitiorDecimationRate);
	if(isFileInput)
		m_pReceiver->setInputFile("signal.log");
	else
		m_pReceiver->setInputSignal(pSignal);

	//Initialize model
	initBusSystem();
	m_pAcquisitior->setMail(m_pMail);
	m_pADC->setMail(m_pMail);
	m_pAGC->setMail(m_pMail);
	m_pVGA->setMail(m_pMail);
	m_pAGC->init();
	m_pAcquisitior->setDESLog("mylog.txt");

	// [ Model parameters
	m_pADC->setLinearRoutine();
	m_pADC->setPrecision(m_pModelParameters->m_uPrecision);
	m_pADC->setExpPosition(m_pModelParameters->m_uExpPosition);
	m_pADC->setConverterMax(m_pModelParameters->m_iADCMax);

	m_pAGC->setPrecision(m_pModelParameters->m_uPrecision);
	m_pAGC->setExpPosition(m_pModelParameters->m_uExpPosition);
	m_pAGC->setConstPrecision(m_pModelParameters->m_uAGCPrecision);
	
	m_pAcquisitior->setAutoCorrelationLength(m_pModelParameters->m_uCorrelationLength);
	m_pAcquisitior->setAutoCorrelationDelay(m_pModelParameters->m_uCorrelationDelay);
	m_pAcquisitior->setWindowSize(m_pModelParameters->m_uAcquisitiorWindowSize);
	// ]
}

void CSynchronizer::initModelParameters(unsigned uPrecision, unsigned uExpPosition, unsigned uAcquisitiorDecimationRate, int iADCMax,
						 unsigned uCorrelationLength, unsigned uCorrelationDelay, unsigned uAcquisitiorWindowSize)
{
	m_pModelParameters->setParameters(uPrecision, uExpPosition, uAcquisitiorDecimationRate, iADCMax, 
		uCorrelationLength, uCorrelationDelay, uAcquisitiorWindowSize);
}

void CSynchronizer::initBusSystem()
{
	//TRACE(_T("CSynchronizer::initBusSystem\n"));

	m_GSLBusMap.insert(pair<string, CBus<CGSLComplex>*  >("Receiver<->VGA",				new CBus<CGSLComplex>()));
	m_GSLBusMap.insert(pair<string, CBus<CGSLComplex>*  >("VGA<->ADC",					new CBus<CGSLComplex>()));
	m_FXPBusMap.insert(pair<string, CBus<CFXPComplex16>*>("ADC<->Splitter",				new CBus<CFXPComplex16>()));
	m_FXPBusMap.insert(pair<string, CBus<CFXPComplex16>*>("Splitter<->AGC",				new CBus<CFXPComplex16>()));
	m_FXPBusMap.insert(pair<string, CBus<CFXPComplex16>*>("Splitter<->Decimator",		new CBus<CFXPComplex16>()));
	m_FXPBusMap.insert(pair<string, CBus<CFXPComplex16>*>("Decimator<->Acquisitior",	new CBus<CFXPComplex16>()));

	// Connect blocks by buses
	m_pReceiver->setOutput(m_GSLBusMap.find("Receiver<->VGA")->second);
	m_pVGA->setInput(m_GSLBusMap.find("Receiver<->VGA")->second);

	m_pVGA->setOutput(m_GSLBusMap.find("VGA<->ADC")->second);
	m_pADC->setInput(m_GSLBusMap.find("VGA<->ADC")->second);

	m_pADC->setOutput(m_FXPBusMap.find("ADC<->Splitter")->second);
	m_pSplitter->setInput(m_FXPBusMap.find("ADC<->Splitter")->second);

	m_pSplitter->registerBusOutput(m_FXPBusMap.find("Splitter<->AGC")->second);
	m_pSplitter->registerBusOutput(m_FXPBusMap.find("Splitter<->Decimator")->second);
	m_pAGC->setInput(m_FXPBusMap.find("Splitter<->AGC")->second);
	m_pDecimator->setInput(m_FXPBusMap.find("Splitter<->Decimator")->second);

	m_pDecimator->setOutput(m_FXPBusMap.find("Decimator<->Acquisitior")->second);
	m_pAcquisitior->setInput(m_FXPBusMap.find("Decimator<->Acquisitior")->second);
}

void CSynchronizer::run(unsigned uSamplesToRead, bool isControlReturn /* = false */)
{
	CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);
	CONSOLESIZE(80, uSamplesToRead*12);

	for(m_iProcessedSamples = 0; m_iProcessedSamples < uSamplesToRead; m_iProcessedSamples++)
	{
		CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_INTENSITY);
		DISPLAY(endl << " --- sample #" << m_iProcessedSamples+1 <<" --- " << endl);
		CONSOLECOLOR(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);

		m_pReceiver->takeControl();
		m_pVGA->takeControl();
		m_pADC->takeControl();
		m_pSplitter->takeControl();
		m_pAGC->takeControl();
		m_pDecimator->takeControl();
		m_pAcquisitior->takeControl();

		if(isControlReturn && m_pAcquisitior->isPacketFound())
			return;
	}

	DISPLAY("\n\n\n");
	DISPLAY("Final situation [Receiver<->VGA] : " << m_GSLBusMap.find("Receiver<->VGA")->second->getSize() << endl);
	DISPLAY("Final situation [VGA<->ADC] : " << m_GSLBusMap.find("VGA<->ADC")->second->getSize() << endl);
	DISPLAY("Final situation [ADC<->Splitter] : " << m_FXPBusMap.find("ADC<->Splitter")->second->getSize() << endl);
	DISPLAY("Final situation [Splitter<->AGC] : " << m_FXPBusMap.find("Splitter<->AGC")->second->getSize() << endl);
	DISPLAY("Final situation [Splitter<->Decimator] : " << m_FXPBusMap.find("Splitter<->Decimator")->second->getSize() << endl);
	DISPLAY("Final situation [Decimator<->Acquisitior] : " << m_FXPBusMap.find("Decimator<->Acquisitior")->second->getSize() << endl);
}

int CSynchronizer::getCurrentGain()
{
	return m_pVGA->getCurrentGain();
}

unsigned CSynchronizer::getCurrentSample()
{
	return m_iProcessedSamples;
}