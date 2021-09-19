# ifndef   MODULATORABL_H
#   define MODULATORABL_H

#include "commonfunc.h"

void modulator_for_ABL( fxp_complex* mod_symbols, 
								        unsigned char* in_bits, 
                        unsigned data_size,
                        unsigned* bits_per_QAM_symbol,
                        unsigned num_of_OFDM_symbols );


#endif // MODULATOR_H


