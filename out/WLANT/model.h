# ifndef   MODEL_H
#   define MODEL_H

//#define LOGGING
//#define TEST_SEQ
//#define AQ_LOG
//#define ABL_TEST
//#define LDPC_TEST
//#define AGC_TEST

#include <gsl/gsl_randist.h>

//[ Random number generator for GSL
extern gsl_rng* RNG;
//]

#ifdef LOGGING
  FILE* debug_log;
#endif

#ifdef LOGGING
  FILE* debug_log;
#endif

#ifdef AQ_LOG
  FILE* aq_log;
#endif

#ifdef AQ_LOG
  FILE* agc_log;
#endif

//char overflow_comment[200];       // Было так
static char overflow_comment[200];


void initialize_model();
void destroy_model();

void logging( char* log_string, char block_begin );

#endif // MODEL_H
