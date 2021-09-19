#include "model.h"
#include "sim_consts.h"
#include <memory.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_complex.h>
#include "commonfunc.h"

/*
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_to_gsl
//
// Purpose: fxp_complex to gsl_complex
//
// Author: Elizar Egorov
//
// Parameters: time_signal - output, Floating-point time domain signal
//
//             fxp_time_signal - input, Fixed-point time domain signal
//
//             num_of_samples - input, Number of samples
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */

static void fxp_to_gsl(gsl_complex* time_signal, fxp_complex* fxp_time_signal, unsigned num_of_samples) // ������ LNK2005 ������������ static
{
    gsl_complex* flp_signal;
    flp_signal = (gsl_complex*)malloc(PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers) * sizeof(gsl_complex));

    std::cout << "\nfxp_to_gsl" << std::endl;
    for (unsigned i = 0; i < num_of_samples * 2; i++)
    {
        flp_signal[i].dat[0] = fxp_time_signal[i].re;
        flp_signal[i].dat[1] = fxp_time_signal[i].im;

        time_signal[i].dat[0] = flp_signal[i].dat[0]; 
        time_signal[i].dat[1] = flp_signal[i].dat[1]; 
    }
    free(flp_signal);
}