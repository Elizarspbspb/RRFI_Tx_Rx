/** 32-bit Fixed point calculations header file
*
* @author: Andrey Efimov, Alexandr Kozlov, Vladislav Chernyshev
* Organization: LCA, ISS Dept, SUAI
* $Revision: 8 $
* $Date: 12.07.05 10:32 $
*/

#include "FXPComplex32.h"

FILE* CFXPComplex32::flog = NULL;

CFXPComplex32::CFXPComplex32(int re /*= 0*/, int im /*= 0*/) : m_Re(re), m_Im(im) {

}

CFXPComplex32::~CFXPComplex32() {

}

/**
* Barrel shifter
* @param shift, input. Number of shifted bits
* @return Result 
*/
CFXPComplex16 CFXPComplex32::operator >> (const int shift) {
	int re = (*this)[cpRe] >> shift;
	int im = (*this)[cpIm] >> shift;

	if (flog)
	{
		if ( ( re < (int)0xFFFF8000 ) || ( re > 0x00007FFF ) )  {
			fprintf( flog, "Overflow in barrel shifter(real part)\n" );
		}

		if ( ( im < (int)0xFFFF8000 ) || ( im > 0x00007FFF ) )  {
			fprintf( flog, "Overflow in barrel shifter(imag part)\n" );
		}
	}

	CFXPComplex16 res( (short int)re, (short int)im );
	return res;
}

/**
* Return real or imag part of complex value
* @param type, input. Complex part (real or imag)
* @return Real or imag part reference
*/
int& CFXPComplex32::operator [] ( ComplexPart type )
{
	if ( type == cpRe )
	{
		return m_Re;
	}
	else
	{
		return m_Im;
	}
}

/**
* Simulate ALU of OFDM Engine
* @param in1, input. 1-st complex argument
* @param in2, input. 2-nd complex argument
* @param cfg, input. ALU mode
* @return Result 32-bit = 16-bit x 16-bit
*/		
CFXPComplex32 CFXPComplex32::ALU(CFXPComplex32 in1, CFXPComplex32 in2, ALUcfg cfg ) {
	CFXPComplex32 res;

	switch( cfg )
	{
	case acReAddImAdd: 
		res[cpRe] = in1[cpRe] + in2[cpRe];
		res[cpIm] = in1[cpIm] + in2[cpIm];
		break;
	case acReSubImSub: 
		res[cpRe] = in1[cpRe] - in2[cpRe];
		res[cpIm] = in1[cpIm] - in2[cpIm];
		break;
	case acReSubImAdd: 
		res[cpRe] = in1[cpRe] - in2[cpRe];
		res[cpIm] = in1[cpIm] + in2[cpIm];
		break;
	case acReAddImSub: 
		res[cpRe] = in1[cpRe] + in2[cpRe];
		res[cpIm] = in1[cpIm] - in2[cpIm];
		break;
	}

	return res;
}

/**
* Complex addition
* @param fxp, input. Complex argument
* @return Result 
*/
CFXPComplex32 CFXPComplex32::operator + (CFXPComplex32 fxp) {
	CFXPComplex32 res;
	res = ALU( (*this), fxp, acReAddImAdd );
	return res;
}

/**
* Complex subtraction
* @param fxp, input. Complex argument
* @return Result 
*/
CFXPComplex32 CFXPComplex32::operator - (CFXPComplex32 fxp) {
	CFXPComplex32 res;
	res = ALU( (*this), fxp, acReSubImSub );
	return res;
}

/**
* Complex unary minus
* @return Result 
*/
CFXPComplex32 CFXPComplex32::operator - () {
	CFXPComplex32 res;
	CFXPComplex32 fxp(0,0);
	res = ALU( fxp, (*this), acReSubImSub );
	return res;
}

/**
* Creates log file for overflow checking
* @param sFileName, input. File name
* @return Result 
*/
void CFXPComplex32::setLogFile(const char *sFileName) {
	if (!flog)
	{
		flog = fopen(sFileName, "w");
	}
}

/**
* Close log file 
*/
void CFXPComplex32::closeLogFile() {
	if (flog)
	{
		fclose(flog);
		flog = NULL;
	}
}