# ifndef   MODULATOR_H
#   define MODULATOR_H

#include "commonfunc.h"

void modulator_11a( fxp_complex* mod_symbols, 
								    unsigned char* in_bits, 
                    unsigned data_size,
                    unsigned bits_per_symbol );




#endif // MODULATOR_H


