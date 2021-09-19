#include "Includes.h"
#include "CSynchronizer.h"
#include "FXPComplex/FXPComplex16.h"

#if !ISLIB

void main()
{
	CSynchronizer Synchronizer;

	Synchronizer.initSynchronizer();
	Synchronizer.run(26600);

	cout << "Well done" << endl;

	while(!kbhit());
}

#endif // if !ISLIB

//extern "C" __declspec(dllexport) void packetDetection(gsl_complex *pSignal, //было
extern "C" void packetDetection(gsl_complex *pSignal, //стало
					 unsigned uNumOfSamples,
					 unsigned *uPacketStartSample,
					 unsigned *uVGAGain,
					 unsigned uPrecision,
					 unsigned uExpPosition,
					 unsigned uAcquisitiorDecimationRate,
					 int iADCMax,
					 unsigned uCorrelationLength,
					 unsigned uCorrelationDelay,
					 unsigned uAcquisitiorWindowSize)
{
	CSynchronizer Synchronizer;

	Synchronizer.initSynchronizer(false, pSignal);
	Synchronizer.initModelParameters(uPrecision, uExpPosition, uAcquisitiorDecimationRate, iADCMax,
		uCorrelationLength, uCorrelationDelay, uAcquisitiorWindowSize);
	Synchronizer.run(uNumOfSamples, true);

	*uPacketStartSample = Synchronizer.getCurrentSample();
	*uVGAGain = Synchronizer.getCurrentGain();
}
