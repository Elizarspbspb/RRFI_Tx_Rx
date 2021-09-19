#ifndef EQUALIZER_H
#define EQUALIZER_H

#include "commonfunc.h"

void equalizer_11a( fxp_complex* freq_OFDM_syms,
		                fxp_complex* channel_estimate,
                    unsigned num_of_OFDM_syms );

#endif // EQUALIZER_H
