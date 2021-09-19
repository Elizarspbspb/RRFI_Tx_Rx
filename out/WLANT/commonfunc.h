# ifndef   COMMONFUNC_H
#   define COMMONFUNC_H

#include <gsl/gsl_complex.h>

unsigned signal_complex_convolution( const gsl_complex* input_signal,
				                             unsigned  num_of_samples,
				                             const gsl_complex* impulse_response, 
				                             unsigned ir_duration, 
				                             gsl_complex* Convolution );

int find_closest_fxp_value( double value, 
                            unsigned precision, 
                            double step );

void get_punc_params( unsigned* punc_pattern, 
                      unsigned* pattern_size,
                      unsigned* punc_length,
                      double code_rate );

void get_PLCP_header( unsigned char* PLCP_header, 
                      unsigned packet_size );

int process_PLCP_header( unsigned* bits_per_QAM_symbol, 
                         unsigned* num_of_bits,
                         double* code_rate,
                         char* useABL,
                         char* useLDPC,
                         unsigned bits_per_OFDM_symbol,
                         unsigned* num_of_pad_bits,
                         unsigned char* PLCP_header );

typedef struct fxpComplex
{
  int re;
  int im;
} fxp_complex;

int convert_flp_to_fxp( double value,
                        unsigned precision,
                        unsigned exp_position,
                        char* overflow_flag,
                        char* overflow_comment );

double convert_fxp_to_flp( int value, 
                           unsigned exp_position );

int convert_precision( int value,
                       unsigned old_precision,
                       unsigned old_exp_position,
                       unsigned new_precision,  
                       unsigned new_exp_position,
                       char* overflow_flag,
                       char* overflow_comment );

int fxp_scaling( int value,
                 unsigned precision,
                 unsigned new_precision,
                 char range_extention_flag,
                 char* overflow_flag,
                 char* overflow_comment );

int make2bit_value( int value );

unsigned get_num_of_used_digits( int value );

int get_bits_from_fxp_number( int value, unsigned first_bits, unsigned last_bits );

int get_least_bits_from_fxp_number( int value, unsigned precision );

int fxp_complex_abs2_beta( fxp_complex a,
                      unsigned precision,
                      char* overflow_flag,
                      char* overflow_comment );

int fxp_add( int a, 
             int b,
             unsigned precision,
             char* overflow_flag,
             char* overflow_comment );

int fxp_sub( int a,
             int b,
             unsigned precision,
             char* overflow_flag, 
             char* overflow_comment );

int fxp_not( int a,
             unsigned precision,
             char* overflow_flag,
             char* overflow_comment );

int fxp_mul( int a,
             int b, 
             unsigned precision,
             unsigned exp_position, 
             char* overflow_flag, 
             char* overflow_comment );

fxp_complex fxp_complex_add( fxp_complex a,
                             fxp_complex b,
                             unsigned precision,
                             char* overflow_flag,
                             char* overflow_comment );

fxp_complex fxp_complex_sub( fxp_complex a,
                             fxp_complex b,
                             unsigned precision, 
                             char* overflow_flag, 
                             char* overflow_comment );

fxp_complex fxp_complex_mul( fxp_complex a,
                             fxp_complex b,
                             unsigned precision,
                             unsigned exp_position, 
                             char* overflow_flag,
                             char* overflow_comment );

fxp_complex fxp_complex_conjugate( fxp_complex a,
                                   unsigned precision, 
                                   char* overflow_flag,
                                   char* overflow_comment );

int fxp_complex_abs2( fxp_complex a,
                      unsigned precision,
											unsigned exp_position,
                      char* overflow_flag,
                      char* overflow_comment );

int fxp_add_angles( int phi1,
                    int phi2,
                    unsigned precision );


int fxp_complex_arg( fxp_complex value,
                     unsigned precision,
                     char* overflow_flag,
                     char* overflow_comment );

fxp_complex fxp_complex_rotate( fxp_complex value,
                                int angle,
                                char* overflow_flag,
                                char* overflow_comment );

/**************** some macroses *******************/
#define SET_FXP_MIN_POSITIVE(result) {result = 1;}
#define SET_FXP_MAX(result,precision) {result = 0; \
                                        result |= ( 1 << ( precision - 1 ) ) - 1;}

#define SET_FXP_MIN(result,precision) {result = 0; \
                                        result |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) \
                                        << ( precision - 1 ); }

/* get sign (-1, +1) */
#define GET_FXP_SIGN(a,precision) (((a) & ( 1 << ( precision - 1 )))?1:-1)
/* greater than zero*/   
#define FXP_GZ(a,precision) (!((a) & (1 << (precision - 1))) && (a))
/* less than zero*/   
#define FXP_LZ(a,precision) ((a) & (1 << (precision - 1)))

/* greater or equal than zero*/   
#define FXP_GEQZ(a,precision) (!((a) & (1 << (precision - 1))))
/* less or equal than zero*/   
#define FXP_LEQZ(a,precision) ( ((a) & (1 << (precision - 1))) || (!a))

/* greater */
#define FXP_L(a,b, precision) (a - b) | ((a | b) & ((a - b) | a))

/**************** end of macroses *******************/

void dump_gsl_complex_array(gsl_complex* data, int data_len, char* filename);
void dump_fxp_complex_array(fxp_complex* data, int data_len, char* filename, char* comment );
void dump_char_array( unsigned char* data, int data_len, char* filename, char* comment );
void dump_fxp_array(int* data, int data_len, char* filename, char* comment );


#endif // COMMONFUNC_H