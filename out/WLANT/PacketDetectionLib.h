#pragma once

extern "C" __declspec(dllexport) void packetDetection(gsl_complex *pSignal, 
													  unsigned uNumOfSamples, 
													  unsigned *uPacketStartSample,
													  unsigned *uVGAGain,
													  unsigned uPrecision, 
													  unsigned uExpPosition, 
													  unsigned uAcquisitiorDecimationRate, 
													  int iADCMax,
													  unsigned uCorrelationLength, 
													  unsigned uCorrelationDelay, 
													  unsigned uAcquisitiorWindowSize);