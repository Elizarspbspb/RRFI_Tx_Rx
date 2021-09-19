#include "ADC.h"
#include "sim_consts.h"
#include "sim_params.h"
#include <math.h>
#include <malloc.h>
#include <stdio.h>

/*
////////////////////////////////////////////////////////////////////////////////
// Name: gsl_to_fxp
//
// Purpose: fxp_complex to gsl_complex
//
// Author: Elizar Egorov
//
// Parameters: fxp_time_signal - output, Floating-point time domain signal
//
//             time_signal - input, Fixed-point time domain signal
//
//             num_of_samples - input, Number of samples
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */

static void gsl_to_fxp(gsl_complex* time_signal, fxp_complex* fxp_time_signal, unsigned num_of_samples)
{
    gsl_complex* flp_signal;
    //flp_signal = (gsl_complex*)malloc(PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers) * sizeof(gsl_complex));
    flp_signal = (gsl_complex*)malloc(65500 * sizeof(gsl_complex));

    std::cout << "\ngsl_to_fxp" << std::endl;

    for (unsigned i = 0; i < num_of_samples; i++)
    {
        flp_signal[i] = time_signal[i];
        fxp_time_signal[i].re = flp_signal[i].dat[0];
        fxp_time_signal[i].im = flp_signal[i].dat[1];
    }
    free(flp_signal);
}