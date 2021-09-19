# ifndef   VGA_H
#   define VGA_H

#include <gsl/gsl_complex.h>

extern int VGA_gain;

extern int curVGA_gain;

void VGA_11a( gsl_complex* time_signal, 
	            unsigned num_of_samples );

#endif // VGA_H
