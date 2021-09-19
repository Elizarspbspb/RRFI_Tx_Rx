#ifndef CHANNELESTIMATION_H
#define CHANNELESTIMATION_H

#include "commonfunc.h"

void channel_estimation_11a( fxp_complex* channel_estimate,
		                         fxp_complex* freq_tr_syms
                             );

#endif // CHANNELESTIMATION_H
