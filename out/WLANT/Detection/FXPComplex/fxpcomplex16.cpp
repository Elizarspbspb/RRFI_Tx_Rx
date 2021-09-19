/** 16-bit Fixed point calculations header file
*
* @author: Andrey Efimov, Alexandr Kozlov, Vladislav Chernyshev
* Organization: LCA, ISS Dept, SUAI
* $Revision: 8 $
* $Date: 12.07.05 10:32 $
*/

#include "FXPComplex16.h"
#include "FXPComplex32.h"

CFXPComplex16::CFXPComplex16(short int re /*= 0*/, short int im /*= 0*/) : m_Re(re), m_Im(im),
m_mult_saturation(false), m_addnsub_round(false), m_mult_round_I(false), m_mult_round_Q(false) {

}


/**
* Sets saturation and rounding params for multiplyers and adders
* @param mult_saturation, input. True - enables multiplyers saturation
* @param addnsub_round, input. Not used now
* @param mult_round_I, input. True - enables multipliers (for real part) rounding
* @param mult_round_Q, input. True - enables multiplyers (for imag part) rounding
*/
void CFXPComplex16::setSatRounding( bool mult_saturation/* = false*/,
								   bool addnsub_round/* = false*/, 
								   bool mult_round_I/* = false*/,
								   bool mult_round_Q/* = false*/	) 
{
	m_mult_saturation = mult_saturation;
	m_addnsub_round = addnsub_round;
	m_mult_round_I = mult_round_I;
	m_mult_round_Q = mult_round_Q;
}

/**
* Return real or imag part of complex value
* @param type, input. Complex part (real or imag)
* @return Real or imag part reference
*/
short int& CFXPComplex16::operator [] ( ComplexPart type )
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
* Complex multiplication 
* @param fxp, input. Complex argument
* @return Result 32-bit = 16-bit x 16-bit
*/
CFXPComplex32 CFXPComplex16::operator * (CFXPComplex16 fxp) {
	return MULL ( *this, fxp, false, m_mult_saturation, 
		m_addnsub_round, false, m_mult_round_I,
		true, m_mult_round_Q );
}

/**
* Complex multiplication with conjucation of fxp
* @param fxp, input. Complex argument
* @return Result 32-bit = 16-bit x 16-bit
*/
CFXPComplex32 CFXPComplex16::operator ^ (CFXPComplex16 fxp) {
	return MULL ( *this, fxp, false, m_mult_saturation, 
		m_addnsub_round, true, m_mult_round_I, false, m_mult_round_Q );
}

/**
* Calculates square absolute value of complex number
* @param fxp, input. Complex argument
* @return Result 32-bit = 16-bit x 16-bit
*/
CFXPComplex32 CFXPComplex16::abs2() {
	CFXPComplex16 fxp(0,0);

	return MULL ( *this, fxp, true, m_mult_saturation, 
		m_addnsub_round, true, m_mult_round_I, true, m_mult_round_Q );		
}

/**
* Simulate MULL of OFDM Engine
* @param in1, input. 1-st complex argument
* @param in2, input. 2-nd complex argument
* @param multiplex, input. Mode of MULL
* @param mult_saturation, input. True - enables multiplyers saturation
* @param addnsub_round, input. Not used now
* @param addnsub_I, input. True - add for real part / False - sub for real part
* @param mult_round_I, input. True - enables multiplyers (for real part) rounding
* @param addnsub_Q, input. True - add for imag part / False - sub for imag part
* @param mult_round_Q, input. True - enables multiplyers (for imag part) rounding
* @return Result 32-bit = 16-bit x 16-bit
*/														 
CFXPComplex32 CFXPComplex16::MULL( CFXPComplex16 in1, CFXPComplex16 in2, 
								  bool multiplex /*= false*/,
								  bool mult_saturation /*= false*/,
								  bool addnsub_round /*= false*/, // not used now
								  bool addnsub_I /*= true*/,
								  bool mult_round_I /*= false*/,
								  bool addnsub_Q /*= true*/,
								  bool mult_round_Q /*= false*/
								  ) 
{
	int I0;
	int I1;

	int Q0;
	int Q1;

	mul( in1, in2, I0, I1, Q0, Q1, multiplex, mult_saturation, mult_round_I, mult_round_Q );

	CFXPComplex32 res = addnsub ( I0, I1, Q0, Q1, addnsub_round, addnsub_I, addnsub_Q );

	return res;
}

/**
* Simulate multiplication part of MULL
* @param in1, input. 1-st complex argument
* @param in2, input. 2-nd complex argument
* @param multiplex, input. Mode of MULL
* @param mult_saturation, input. True - enables multiplyers saturation
* @param mult_round_I, input. True - enables multiplyers (for real part) rounding
* @param mult_round_Q, input. True - enables multiplyers (for imag part) rounding
* @return Result 32-bit = 16-bit x 16-bit
*/		
void CFXPComplex16::mul (  CFXPComplex16 in1, CFXPComplex16 in2, 
						 int& I0,
						 int& I1,
						 int& Q0,
						 int& Q1,
						 bool multiplex/* = false*/,
						 bool mult_saturation/* = false*/,
						 bool mult_round_I/* = false*/,
						 bool mult_round_Q/* = false*/
						 ) 
{
	short int i000 =						in1[cpRe];           
	short int i001 = multiplex ? in1[cpRe]:	in2[cpRe]; 
	short int i010 =						in1[cpIm];           
	short int i011 = multiplex ? in1[cpIm]:	in2[cpIm]; 
	short int i100 = multiplex ? in2[cpRe]:	in1[cpIm]; 
	short int i101 =						in2[cpRe];           
	short int i110 = multiplex ? in2[cpIm]:	in1[cpRe]; 
	short int i111 =						in2[cpIm];           


	I0 = Q1_15mul( i000, i001, mult_saturation, mult_round_I );
	I1 = Q1_15mul( i010, i011, mult_saturation, mult_round_I );

	Q0 = Q1_15mul( i100, i101, mult_saturation, mult_round_Q );
	Q1 = Q1_15mul( i110, i111, mult_saturation, mult_round_Q );
}

/**
* Simulate addition/substraction part of MULL
* @param in1, input. 1-st complex argument
* @param in2, input. 2-nd complex argument
* @param addnsub_round, input. Not used now
* @param addnsub_I, input. True - add for real part / False - sub for real part
* @param addnsub_Q, input. True - add for imag part / False - sub for imag part
* @return Result 32-bit = 16-bit x 16-bit
*/	
CFXPComplex32	CFXPComplex16::addnsub ( 
										int I0,
										int I1,
										int Q0,
										int Q1,
										bool addnsub_round /*= false*/, // not used now
										bool addnsub_I /*= true*/,
										bool addnsub_Q /*= true*/
										) 
{
	CFXPComplex32 res;

	res[cpRe] = addnsub_I ? (I0 + I1) : (I0 - I1);
	res[cpIm] = addnsub_Q ? (Q0 + Q1) : (Q0 - Q1);

	return res;
}

/**
* Simulate Q1.15 multiplication with rounding and saturation
* @param dataa, input. 1-st 16-bit argument
* @param datab, input. 2-nd 16-bit argument
* @param saturation, input. True - enables saturation
* @param rounding, input. True - enables rounding
* @return Result 32-bit = 16-bit x 16-bit
*/	
int CFXPComplex16::Q1_15mul( short int dataa, short int datab, bool saturation, bool rounding ) 
{
	int a = dataa;
	int b = datab;

	int r;

	if ( saturation && ( a == 0xFFFF8000 ) && ( b == 0xFFFF8000 ) )
	{
		if ( rounding )
			r	= 0x3FFF8000; 
		else
			r = 0x3FFFFFFF; 
	}
	else	
	{
		if ( rounding )
			r = ((a * b) + (1 << 14)) & 0xFFFF8000;
		else
			r = a * b;  
	}

	return r;
}

/**
* Converts floating point value to fixed point value
* @param re, input. floating point value (real part)
* @param im, input. floating point value (real part)
* @param precision, input. Fixed-point precision 
* @param exp_position, input. Exponent point position, for example: 1010.10 exp_position = 2
*/	
void CFXPComplex16::setFromFlp( double re, double im, unsigned precision, unsigned exp_position ) {
	int fxp = 0;

	m_Re = convert_flp_to_fxp( re, precision, exp_position );
	m_Im = convert_flp_to_fxp( im, precision, exp_position );
}

/**
* Converts floating point value to fixed point value
* @param value, input. floating point value 
* @param precision, input. Fixed-point precision 
* @param exp_position, input. Exponent point position, for example: 1010.10 exp_position = 2
*/	
short int CFXPComplex16::convert_flp_to_fxp( double value, 
											unsigned precision,
											unsigned exp_position )
{
	short int fxp = 0;

	if ( value < -( 1 << ( precision - exp_position - 1 ) ) )
	{
		//[ Set fixed-point number to minimum, due to downflow
		fxp |= ( ( 1 << ( 16 - precision + 1 ) ) - 1 ) << ( precision - 1 );
		//]
	}
	else if ( value > ( 1 << ( precision - exp_position - 1 ) ) -
		1.0 / ( 1 << exp_position ) ) 
	{
		//[ Set fixed-point number to maximum, due to overflow
		fxp |= ( 1 << ( precision - 1 ) ) - 1;
		//]
	}
	else
	{
		//[ Convert floating-point number to fixed-point number
		fxp = (short int)( value * ( 1 << exp_position ) );
		//]
	}

	return fxp;
}