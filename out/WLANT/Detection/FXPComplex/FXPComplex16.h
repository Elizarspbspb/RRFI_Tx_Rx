/** 16-bit Fixed point calculations header file
*
* @author: Andrey Efimov, Alexandr Kozlov, Vladislav Chernyshev
* Organization: LCA, ISS Dept, SUAI
* $Revision: 8 $
* $Date: 12.07.05 10:32 $
*/

#pragma once

#include "../Includes.h"
#include "FXPComplex32.h"

class CFXPComplex32;

/// 16-bit Fixed point calculations
class CFXPComplex16 {
public:
	CFXPComplex16(short int re = 0, short int im = 0);

public:
	short int& operator [] ( ComplexPart type );

	void setFromFlp( double re, double im, unsigned precision, unsigned exp_position );

	CFXPComplex32 operator * (CFXPComplex16 fxp);

	CFXPComplex32 operator ^ (CFXPComplex16 fxp);

	CFXPComplex32 abs2();

	void setSatRounding( bool mult_saturation = false,
		bool addnsub_round = false, // not used now
		bool mult_round_I = false,
		bool mult_round_Q = false	);

	static CFXPComplex32 MULL( CFXPComplex16 in1, CFXPComplex16 in2, 
		bool multiplex = false,
		bool mult_saturation = false,
		bool addnsub_round = false, // not used now
		bool addnsub_I = true,
		bool mult_round_I = false,
		bool addnsub_Q = true,
		bool mult_round_Q = false
		);

	static void	mul ( CFXPComplex16 in1, CFXPComplex16 in2, 
		int& I0,
		int& I1,
		int& Q0,
		int& Q1,
		bool multiplex = false,
		bool mult_saturation = false,
		bool mult_round_I = false,
		bool mult_round_Q = false
		);

	static CFXPComplex32 addnsub (
		int I0,
		int I1,
		int Q0,
		int Q1,
		bool addnsub_round = false, // not used now
		bool addnsub_I = true,
		bool addnsub_Q = true
		);

protected:
	static int Q1_15mul( short int dataa, short int datab, bool saturation, bool rounding );

	static short int convert_flp_to_fxp( double value, 
		unsigned precision,
		unsigned exp_position );

	short int m_Re;
	short int m_Im; 

	bool m_mult_saturation;
	bool m_addnsub_round;
	bool m_mult_round_I;
	bool m_mult_round_Q;

};
