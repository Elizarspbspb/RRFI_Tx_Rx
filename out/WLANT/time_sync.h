# ifndef   TIMESYNC_H
#   define TIMESYNC_H

#include "commonfunc.h"

void init_timing_matched_filter();

unsigned fine_timing_sync_11a( fxp_complex* time_signal );

#endif 