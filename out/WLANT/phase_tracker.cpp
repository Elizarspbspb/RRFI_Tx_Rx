#include "phase_tracker.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "model.h"

#include <gsl/gsl_complex_math.h>

/**
////////////////////////////////////////////////////////////////////////////////
// Name: phase_tracker_11a
//
// Purpose: Implements phase offset correction
//
// Author: Alexandr Kozlov
//
// Parameters: freq_OFDM_syms - input/output, OFDM symbols
//																
//             num_of_OFDM_syms - input, Number of OFDM symbols
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void phase_tracker_11a( fxp_complex* freq_OFDM_sym,
                        int current_sym_correction,
                        int current_subc_correction,
                        int *estimated_sym_correction,
                        int *estimated_subc_correction,
                        int *rotor_correction,
                        int OFDM_sym_number,
                        char first )
{
  unsigned j;
  fxp_complex correction_sym;

  char overflow_flag;

  double angle2;
  gsl_complex rot;
  gsl_complex temp_sym;
  double a;
  double b;
  gsl_complex pilots[4];

  char old_phase = 1;


  #ifdef LOGGING
  logging( "phase_tracker_11a...\n", 1 );
  #endif

  correction_sym.re = 0;
  correction_sym.im = 0;

  //[ Estimate phase error
  for( j = 0; j < NumOfPilotSubcarriers; j++ )
	{
    if ( sim_consts.PilotScramble[OFDM_sym_number%127] * sim_consts.PilotSubcSymbols[j] > 0 )
    {
      correction_sym = 
        fxp_complex_add( correction_sym,
                         freq_OFDM_sym[sim_consts.PilotSubcIdx[j] - 1],
                         fxp_params.Demodulator_precision,
                         &overflow_flag,
                         overflow_comment );

      pilots[j].dat[0] = freq_OFDM_sym[sim_consts.PilotSubcIdx[j] - 1].re;
      pilots[j].dat[1] = freq_OFDM_sym[sim_consts.PilotSubcIdx[j] - 1].im;
    }
    else
    {
      correction_sym = 
        fxp_complex_sub( correction_sym,
                         freq_OFDM_sym[sim_consts.PilotSubcIdx[j] - 1],
                         fxp_params.Demodulator_precision,
                         &overflow_flag,
                         overflow_comment );

      pilots[j].dat[0] = -freq_OFDM_sym[sim_consts.PilotSubcIdx[j] - 1].re;
      pilots[j].dat[1] = -freq_OFDM_sym[sim_consts.PilotSubcIdx[j] - 1].im;
    }
  }

  *estimated_subc_correction = 
      find_closest_fxp_value(
        gsl_complex_arg( 
            gsl_complex_add(
              gsl_complex_mul( pilots[2], gsl_complex_conjugate( pilots[0] ) ),
              gsl_complex_mul( pilots[3], gsl_complex_conjugate( pilots[1] ) ) )
          ) / 28.0,
        fxp_params.Arctan_precision,
        2 * Pi / ( 1 << fxp_params.Arctan_precision ) );

  *estimated_sym_correction = old_phase ? -fxp_complex_arg( correction_sym,
                                                fxp_params.Arctan_precision,
                                                &overflow_flag, 
                                                overflow_comment ) :
      find_closest_fxp_value(
        -gsl_complex_arg( 
          gsl_complex_add(
            gsl_complex_mul( pilots[0], pilots[3] ),
            gsl_complex_mul( pilots[1], pilots[2] ) )
          ) / 2.0,
        fxp_params.Arctan_precision,
        2 * Pi / ( 1 << fxp_params.Arctan_precision ) );

  *rotor_correction = old_phase ? 0 :
    find_closest_fxp_value(
     -gsl_complex_arg( 
        gsl_complex_add(
          gsl_complex_mul( pilots[0], pilots[3] ),
          gsl_complex_mul( pilots[1], pilots[2] ) )
        ) / 2.0 / 80.0,
     fxp_params.Arctan_precision,
     2 * Pi / ( 1 << fxp_params.Arctan_precision ) );
  //]

  //[ Correct phase error
  if (!first)
  {
 /*   for( j = 0; j < NumOfUsedSubcarriers; j++ )
    {
      freq_OFDM_sym[sim_consts.UsedSubcIdx[j] - 1] = 
        fxp_complex_rotate( freq_OFDM_sym[sim_consts.UsedSubcIdx[j] - 1],
                            current_sym_correction,
                            &overflow_flag,
                            overflow_comment );
    }*/
    b = current_sym_correction * ( 2.0 * Pi / ( 1 << fxp_params.Arctan_precision ) );
    a = current_subc_correction * ( 2.0 * Pi / ( 1 << fxp_params.Arctan_precision ) );

    for( j = 0; j < NumOfUsedSubcarriers; j++ )
    {
      angle2 = old_phase ? current_sym_correction * ( 2.0 * Pi / ( 1 << fxp_params.Arctan_precision ) ) :
               ( ( (double)j - 26 ) * a );

      rot = gsl_complex_exp( gsl_complex_rect( 0, angle2 ) );

      temp_sym.dat[0] = freq_OFDM_sym[sim_consts.UsedSubcIdx[j] - 1 ].re;
      temp_sym.dat[1] = freq_OFDM_sym[sim_consts.UsedSubcIdx[j] - 1 ].im;

      temp_sym = gsl_complex_mul( temp_sym, rot );   

      freq_OFDM_sym[sim_consts.UsedSubcIdx[j] - 1].re = (int) temp_sym.dat[0];
      freq_OFDM_sym[sim_consts.UsedSubcIdx[j] - 1].im = (int) temp_sym.dat[1];
    }
  }
  //]

  #ifdef LOGGING
  logging( "phase_tracker_11a finished\n", -1 );
  #endif
}
