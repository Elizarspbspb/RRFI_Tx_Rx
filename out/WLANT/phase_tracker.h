# ifndef   PHASETRACKER_H
#   define PHASETRACKER_H

#include "commonfunc.h"

void phase_tracker_11a( fxp_complex* freq_OFDM_sym,
                        int current_sym_correction,
                        int current_subc_correction,
                        int *estimated_sym_correction,
                        int *estimated_subc_correction,
                        int *rotor_correction,
                        int OFDM_sym_number,
                        char first );

#endif 