# ifndef   TRANSMITTER_H
#   define TRANSMITTER_H

#include <gsl/gsl_complex.h>

unsigned transmitter( gsl_complex* time_signal,
                      unsigned char* inf_bits, 
                      unsigned packet_size,
                      unsigned* ABL_bits_per_QAM_symbol,
                      unsigned bits_per_OFDM_symbol );

#endif // TRANSMITTER_H