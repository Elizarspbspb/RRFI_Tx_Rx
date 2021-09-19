# ifndef   CHANNEL_H
#   define CHANNEL_H

#include <gsl/gsl_complex.h>
#include "sim_consts.h"

#define NOISE_POWER (-100) // dBm/Hz

unsigned get_channel_ir( gsl_complex* cir );
unsigned get_channel_ir_len();

unsigned channel( gsl_complex* received_signal, 
		              gsl_complex* time_signal, 
 		              gsl_complex* cir, 
		              unsigned num_of_samples, 
		              unsigned ir_duration,
		              double SNR );

void create_freq_offset( gsl_complex* input_signal, unsigned num_of_samples );


///////////////////////////////////////////////////////////////////////////////////////////////////////
// AWGN Channel Wrapper functions
unsigned get_AWGN_channel_ir( gsl_complex* cir );
unsigned get_AWGN_channel_ir_len();


///////////////////////////////////////////////////////////////////////////////////////////////////////
// DopplerChannel Wrapper functions


int initDopplerChannel(
    char                       *channelFileName ,
    char                       *channelModelName               
 );
int destroyDopplerChannel();

unsigned get_dopplerChannel_ir( gsl_complex* cir );
unsigned get_dopplerChannel_ir_len();

unsigned dopplerChannel( gsl_complex* received_signal, 
		              gsl_complex* time_signal, 
 		              gsl_complex* cir, 
		              unsigned num_of_samples, 
		              unsigned ir_duration,
		              double SNR );


#endif // CHANNEL_H