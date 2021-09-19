#ifndef REMOVECYCLICPREFIX_H
#define REMOVECYCLICPREFIX_H

#include "commonfunc.h"

void remove_cyclic_prefix_11a( fxp_complex* time_OFDM_symbols,
		                           fxp_complex* time_signal,
                               unsigned num_of_symbols );

#endif // REMOVECYCLICPREFIX_H
