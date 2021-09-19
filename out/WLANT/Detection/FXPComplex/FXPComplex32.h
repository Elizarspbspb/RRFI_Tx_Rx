/** 32-bit Fixed point calculations header file
*
* @author: Andrey Efimov, Alexandr Kozlov, Vladislav Chernyshev
* Organization: LCA, ISS Dept, SUAI
* $Revision: 8 $
* $Date: 12.07.05 10:32 $
*/

#pragma once

// [ Import declarations ]
#include "../Includes.h"
#include "FXPComplex16.h"

class CFXPComplex16;

/// 32-bit Fixed point calculations
class CFXPComplex32 {
public:
	CFXPComplex32(int re = 0, int im = 0);
	~CFXPComplex32();  

public:
	CFXPComplex16 operator >> (const int shift);

	int& operator [] ( ComplexPart type );

	static CFXPComplex32 ALU(CFXPComplex32 in1, CFXPComplex32 in2, ALUcfg cfg );

	CFXPComplex32 operator + (CFXPComplex32 fxp);

	CFXPComplex32 operator - (CFXPComplex32 fxp);

	CFXPComplex32 operator - ();

	static void setLogFile(const char *sFileName);
	static void closeLogFile();


protected:
	int m_Re;
	int m_Im; 

	static FILE* flog;   
};
