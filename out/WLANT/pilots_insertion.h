# ifndef   PILOTS_H
#   define PILOTS_H

#include "commonfunc.h"

void insert_pilot_symbols_11a( fxp_complex* ofdm_symbols,
											         fxp_complex* mod_symbols, 
                               unsigned num_of_OFDM_symbols );

#endif