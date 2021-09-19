#pragma once
#include "Includes.h" //было
//#include "/home/elizar/Code_Blocks/WLAN Transmitter/Detection/Includes.h"

//!Class, warp to complex template
class CGSLComplex : public complex<double>
{
public:
	CGSLComplex(double dReal, double dImg) : complex<double>(dReal, dImg) {};
};
