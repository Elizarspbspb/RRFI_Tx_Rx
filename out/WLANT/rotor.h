# ifndef   ROTOR_H
#   define ROTOR_H

#include "commonfunc.h"

static int rotor_state_angle;

void rotor_11a( fxp_complex* time_signal, 
                int radians_per_sample,
                unsigned num_of_samples );

#endif // ROTOR_H